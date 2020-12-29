
#include "mods/utils/arithmeticShifter.hpp"
#include "mods/wav/ADPCMDecoderConverter.hpp"

#include <iostream>
#include <limits>

namespace mods
{
   namespace wav
     {
        template<int NB_CHANNELS>
          ADPCMDecoderConverter<NB_CHANNELS>::ADPCMDecoderConverter(WavConverter::ptr src, const Format& format)
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
                    std::cout << "Warning: ADPCMDecoderConverter: stream is not 4 bits per sample" << std::endl;
                 }
               
               if(_blockSize != format.getBitsPerContainer() * NB_CHANNELS / BITS_IN_BYTE)
                 {
                    std::cout << "Warning: ADPCMDecoderConverter: blocks contain padding" << std::endl;
                 }
               
               if(_extension->getNumCoef() * 2 < _coefs.size())
                 {
                    _coefs = _coefs.slice<s16>(0, _extension->getNumCoef() * 2);
                 }
	    }
        
        template<int NB_CHANNELS>
          auto ADPCMDecoderConverter<NB_CHANNELS>::initBlockSize() -> u32
          {
             auto samplesPerBlock = _extension->getSamplesPerBlock();
             
             if((samplesPerBlock & 1U) != 0)
               {
                  std::cout << "Warning: ADPCMDecoderConverter: samples per block is odd" << std::endl;
               }
             if(samplesPerBlock <= 2)
               {
                  std::cout << "Warning: ADPCMDecoderConverter: samples per block too small" << std::endl;
               }
             
             auto samplesAfterPreamble = samplesPerBlock - 2;
             
             return samplesAfterPreamble / 2 * NB_CHANNELS + sizeof(Preamble);
          }
        
        template<int NB_CHANNELS>
          auto ADPCMDecoderConverter<NB_CHANNELS>::allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>
          {
             _encodedVec.resize(len);
             u8* ptr = _encodedVec.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
             return mods::utils::RWBuffer<u8>(buffer);
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
	  void ADPCMDecoderConverter<NB_CHANNELS>::read(mods::utils::RWBuffer<u8>* buf, size_t len)
	    {
	       if((len & 1U) != 0)
		 {
		    std::cout << "Odd length in ADPCM not supported" << std::endl;
		 }
	       
	       size_t count = 0;
	       size_t nbElems = len / 2;
	       auto out = buf->slice<s16>(0, nbElems);
	       
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
			      _src->read(&_encodedBuffer, _blockSize);
			      for(int i=0; i<NB_CHANNELS; ++i)
				{
				   _decoders[i].initDecoder(_preamble->getSampleTMinus1(i), _preamble->getSampleTMinus2(i), _preamble->getBlockPredictor(i), _coefs, _preamble->getInitialDelta(i));
				}
			      _itDataBuffer = _dataBuffer.RBuffer<u8>::begin();
			      
			      for(int i=0; i<NB_CHANNELS; ++i) 
				{
				   out[count++] = _decoders[i].getSampleTMinus2();
				}
			      for(int i=0; i<NB_CHANNELS; ++i) 
				{
				   out[count++] = _decoders[i].getSampleTMinus1();
				}
			   }
			 else
			   {
			      out[count++] = 0;
			   }
		      }
		    else
		      {
			 u8 v = *_itDataBuffer;
			 int sample = static_cast<u8>(v >> 4U) & nibbleMask;
			 _nextSample = _decoders[0].decodeSample(sample);
			 out[count++] = _nextSample;
			 
			 sample = v & nibbleMask;
			 if(NB_CHANNELS == 1)
			   {
			      _nextSample = _decoders[0].decodeSample(sample);
			   }
			 else
			   {
			      _nextSample = _decoders[1].decodeSample(sample);
			   }
			 _sampleAvailable = true;
			 
			 ++_itDataBuffer;
		      }
		 }
	    }
	
	namespace impl
	  {
	     void ADPCMChannelDecoder::initDecoder(s16 sampleTMinus1, s16 sampleTMinus2, size_t predictor, const mods::utils::RBuffer<s16>& coefs, s16 initialDelta)
	       {
		  using mods::utils::at;
		  
		  _sampleTMinus1 = sampleTMinus1;
		  _sampleTMinus2 = sampleTMinus2;
		  
		  if(predictor * 2 + 1 < coefs.size())
		    {
		       _coef1 = coefs[predictor * 2];
		       _coef2 = coefs[predictor * 2 + 1];
		    }
		  else if(predictor * 2 + 1 < defaultCoefs.size())
		    {
		       _coef1 = at(defaultCoefs, predictor * 2);
		       _coef2 = at(defaultCoefs, predictor * 2 + 1);
		    }
		  else
		    {
		       std::cout << "Warning: invalid predictor: " << predictor << std::endl;
		    }
		  
		  _delta = initialDelta;
	       }
	     
	     auto ADPCMChannelDecoder::getSampleTMinus2() -> s16
	       {
		  return _sampleTMinus2;
	       }
	     
	     auto ADPCMChannelDecoder::getSampleTMinus1() -> s16
	       {
		  return _sampleTMinus1;
	       }
	     
	     auto ADPCMChannelDecoder::decodeSample(u8 sample) -> s16
	       {
		  using mods::utils::at;
		  
		  static constexpr s32 fixedPointCoefBase = 256;
		  static constexpr s32 fixedPointAdaptationBase = 256;
		  static constexpr u32 nibleSignExtensionShift = 28U;
		  
		  s32 errorDelta = static_cast<u32>(sample) << nibleSignExtensionShift;
		  errorDelta = mods::utils::arithmeticShifter::shiftRight(errorDelta, nibleSignExtensionShift);
		  
		  s32 predSample = (static_cast<s32>(_sampleTMinus1) * _coef1 + static_cast<s32>(_sampleTMinus2) * _coef2) / fixedPointCoefBase;
		  s32 nextSample = predSample + _delta * errorDelta;
		  
		  nextSample = mods::utils::clamp(nextSample, 
						  static_cast<int>(std::numeric_limits<s16>::min()),
						  static_cast<int>(std::numeric_limits<s16>::max()));
		  
		  _delta = _delta * at(adaptationTable,sample) / fixedPointAdaptationBase;
		  
		  if(_delta < 16) 
		    {
		       _delta = 16;
		    }
		  
		  _sampleTMinus2 = _sampleTMinus1;
		  _sampleTMinus1 = nextSample;
		  
		  return nextSample;
	       }
	  } // namespace impl
        
        template class ADPCMDecoderConverter<1>;
        template class ADPCMDecoderConverter<2>;
     } // namespace wav
} // namespace mods
