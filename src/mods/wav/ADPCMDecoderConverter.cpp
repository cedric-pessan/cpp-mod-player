
#include "mods/converters/Converter.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/RWBufferBackend.hpp"
#include "mods/utils/arithmeticShifter.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/ADPCMDecoderConverter.hpp"
#include "mods/wav/Format.hpp"
#include "mods/wav/impl/ADPCMDecoderConverterImpl.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>

namespace mods
{
   namespace wav
     {
        template<int NB_CHANNELS>
          ADPCMDecoderConverter<NB_CHANNELS>::ADPCMDecoderConverter(Converter<u8>::ptr src, const Format& format)
            : _src(std::move(src)),
          _extension(format.getMetaData().slice<impl::ADPCMExtension>(0,1)),
          _coefs(format.getMetaData().slice<s16>(sizeof(impl::ADPCMExtension), (format.getMetaData().size() - sizeof(impl::ADPCMExtension)) / sizeof(s16))),
          _blockSize(initBlockSize()),
          _encodedBuffer(allocateNewTempBuffer(_blockSize)),
          _dataBuffer(_encodedBuffer.readOnlySlice<u8>(sizeof(Preamble), _blockSize - sizeof(Preamble))),
          _itDataBuffer(_dataBuffer.end()),
          _preamble(_encodedBuffer.readOnlySlice<Preamble>(0, 1))
	    {
               if(format.getBitsPerSample() != 4)
                 {
                    std::cout << "Warning: ADPCMDecoderConverter: stream is not 4 bits per sample" << '\n';
                 }
               
               if(_blockSize != format.getBitsPerContainer() * NB_CHANNELS / BITS_IN_BYTE)
                 {
                    std::cout << "Warning: ADPCMDecoderConverter: blocks contain padding" << '\n';
                 }
               
               if(static_cast<u64>(_extension->getNumCoef() * 2) < _coefs.size())
                 {
                    _coefs = _coefs.slice<s16>(0, _extension->getNumCoef() * 2UL);
                 }
	    }
        
        template<int NB_CHANNELS>
          auto ADPCMDecoderConverter<NB_CHANNELS>::initBlockSize() -> u32
          {
             auto samplesPerBlock = _extension->getSamplesPerBlock();
             
             if((samplesPerBlock & 1U) != 0)
               {
                  std::cout << "Warning: ADPCMDecoderConverter: samples per block is odd" << '\n';
               }
             if(samplesPerBlock <= 2)
               {
                  std::cout << "Warning: ADPCMDecoderConverter: samples per block too small" << '\n';
               }
             
             auto samplesAfterPreamble = samplesPerBlock - 2;
             
             return (samplesAfterPreamble / 2UL * NB_CHANNELS) + sizeof(Preamble);
          }
        
        template<int NB_CHANNELS>
          auto ADPCMDecoderConverter<NB_CHANNELS>::allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>
          {
             _encodedVec.resize(len);
             u8* ptr = _encodedVec.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
             return mods::utils::RWBuffer<u8>(std::move(buffer));
          }
	
        template<int NB_CHANNELS>
          auto ADPCMDecoderConverter<NB_CHANNELS>::isFinished() const -> bool
	  {
             return !_sampleAvailable && _itDataBuffer >= _dataBuffer.end() && _src->isFinished();
	  }
        
        namespace
          {
             constexpr std::array<s16, 14> defaultCoefs
               {
                  256, 0,
                    512, -256,
                    0, 0,
                    192, 64,
                    240, 0,
                    460, -208,
                    392, -232
               };
             
             constexpr std::array<s16, 16> adaptationTable
               {
                  230, 230, 230, 230, 307, 409, 512, 614, 
                    768, 614, 512, 409, 307, 230, 230, 230 
               };
          } // namespace
	
	template<int NB_CHANNELS>
	  void ADPCMDecoderConverter<NB_CHANNELS>::read(mods::utils::RWBuffer<s16>* buf)
	    {
               using mods::utils::at;
	       
	       size_t count = 0;
	       const size_t nbElems = buf->size();
	       auto& out = *buf;
	       
	       static constexpr u8 nibbleMask = 0xFU;
	       
	       while(count < nbElems)
		 {
		    if(_sampleAvailable)
		      {
			 out[count++] = _nextSample;
			 _sampleAvailable = false;
		      }
		    else if(_itDataBuffer == _dataBuffer.RBuffer<u8>::end())
		      {
			 if(!isFinished())
			   {
			      _src->read(&_encodedBuffer);
			      for(int i=0; i<NB_CHANNELS; ++i)
				{
				   at(_decoders, i).initDecoder(impl::ChannelPreamble{_preamble->getSampleTMinus1(i), _preamble->getSampleTMinus2(i), _preamble->getBlockPredictor(i), _preamble->getInitialDelta(i)}, _coefs);
				}
			      _itDataBuffer = _dataBuffer.RBuffer<u8>::begin();
			      
			      for(int i=0; i<NB_CHANNELS; ++i) 
				{
				   out[count++] = at(_decoders, i).getSampleTMinus2();
				}
			      for(int i=0; i<NB_CHANNELS; ++i) 
				{
				   out[count++] = at(_decoders, i).getSampleTMinus1();
				}
			   }
			 else
			   {
			      out[count++] = 0;
			   }
		      }
		    else
		      {
			 const u8 val = *_itDataBuffer;
			 int sample = static_cast<u8>(val >> 4U) & nibbleMask;
			 _nextSample = _decoders[0].decodeSample(sample);
			 out[count++] = _nextSample;
			 
			 sample = val & nibbleMask;
                         _nextSample = _decoders.back().decodeSample(sample);
			 _sampleAvailable = true;
			 
			 ++_itDataBuffer;
		      }
		 }
	    }
	
	namespace impl
	  {
	     void ADPCMChannelDecoder::initDecoder(const ChannelPreamble& channelPreamble, const mods::utils::RBuffer<s16>& coefs)
	       {
		  using mods::utils::at;
		  
		  _sampleTMinus1 = channelPreamble.sampleTMinus1;
		  _sampleTMinus2 = channelPreamble.sampleTMinus2;
                  const auto predictor = channelPreamble.predictor;
                  const auto initialDelta = channelPreamble.initialDelta;
		  
		  if(predictor * 2 + 1 < coefs.size())
		    {
		       _coef1 = coefs[predictor * 2];
		       _coef2 = coefs[(predictor * 2) + 1];
		    }
		  else if(predictor * 2 + 1 < defaultCoefs.size())
		    {
		       _coef1 = at(defaultCoefs, predictor * 2);
		       _coef2 = at(defaultCoefs, (predictor * 2) + 1);
		    }
		  else
		    {
		       std::cout << "Warning: invalid predictor: " << predictor << '\n';
		    }
		  
		  _delta = initialDelta;
	       }
	     
	     auto ADPCMChannelDecoder::getSampleTMinus2() const -> s16
	       {
		  return _sampleTMinus2;
	       }
	     
	     auto ADPCMChannelDecoder::getSampleTMinus1() const -> s16
	       {
		  return _sampleTMinus1;
	       }
	     
	     auto ADPCMChannelDecoder::decodeSample(u8 sample) -> s16
	       {
		  using mods::utils::at;
                  using mods::utils::arithmeticShifter::Shift;
		  
		  static constexpr s32 fixedPointCoefBase = 256;
		  static constexpr s32 fixedPointAdaptationBase = 256;
		  static constexpr u32 nibleSignExtensionShift = 28U;
		  
		  s32 errorDelta = static_cast<s32>(static_cast<u32>(sample) << nibleSignExtensionShift);
		  errorDelta = mods::utils::arithmeticShifter::shiftRight(errorDelta, static_cast<Shift>(nibleSignExtensionShift));
		  
		  const s32 predSample = (static_cast<s32>(_sampleTMinus1) * _coef1 + static_cast<s32>(_sampleTMinus2) * _coef2) / fixedPointCoefBase;
		  s32 nextSample = predSample + (_delta * errorDelta);
		  
		  nextSample = mods::utils::clamp(nextSample, 
						  static_cast<int>(std::numeric_limits<s16>::min()),
						  static_cast<int>(std::numeric_limits<s16>::max()));
		  
		  _delta = static_cast<s16>(_delta * at(adaptationTable,sample) / fixedPointAdaptationBase);
		  
                  static constexpr s16 MIN_ALLOWED_DELTA = 16;
                  
                  _delta = std::max(_delta, MIN_ALLOWED_DELTA);
		  
		  _sampleTMinus2 = _sampleTMinus1;
		  _sampleTMinus1 = static_cast<s16>(nextSample);
		  
		  return static_cast<s16>(nextSample);
	       }
	  } // namespace impl
        
        template class ADPCMDecoderConverter<1>;
        template class ADPCMDecoderConverter<2>;
     } // namespace wav
} // namespace mods
