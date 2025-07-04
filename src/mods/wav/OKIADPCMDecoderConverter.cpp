
#include "mods/converters/Converter.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/RWBufferBackend.hpp"
#include "mods/utils/at.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/Format.hpp"
#include "mods/wav/OKIADPCMDecoderConverter.hpp"

#include <array>
#include <cstddef>
#include <iostream>
#include <memory>
#include <sys/types.h>
#include <utility>

namespace mods
{
   namespace wav
     {
	OKIADPCMDecoderConverter::OKIADPCMDecoderConverter(Converter<u8>::ptr src, const Format& format)
	  : _src(std::move(src)),
          _nbChannels(format.getNumChannels()),
	  _encodedBuffer(allocateNewTempBuffer(0))
	    {
               if(format.getBitsPerSample() == 3)
                 {
                    std::cout << "Warning: OKIADPCMDecoderConverter: stream is 3 bits per sample" << '\n';
                 }
               
               if(format.hasMetaData())
                 {
                    std::cout << "Warning: OKIADPCMDecoderConverter: wav have extension (pole?)" << '\n';
                 }
               
               for(int i=0; i<_nbChannels; ++i)
                 {
                    _decoders.emplace_back(i, _encodedBuffer, _nbChannels);
                 }
	    }
        
        namespace
	  {
	     constexpr std::array<int, 16> indexTable
	       {
		  -1, -1, -1, -1, 2, 4, 6, 8,
		    -1, -1, -1, -1, 2, 4, 6, 8
	       };
	     
	     constexpr std::array<int, 49> stepSizeTable
	       {
		  16, 17, 19, 21, 23, 25, 31, 34, 37, 41, 45,
		    50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143,
		    157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449,
		    494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552
	       };
	  } // namespace
	
	auto OKIADPCMDecoderConverter::isFinished() const -> bool
	  {
             for(const auto& decoder : _decoders)
               {
                  if(!decoder.hasNextSampleInCurrentBlock())
                    {
                       return false;
                    }
               }
             
             return _src->isFinished();
	  }
	
	void OKIADPCMDecoderConverter::read(mods::utils::RWBuffer<s16>* buf)
	  {
	     if((buf->size() & 1U) != 0)
	       {
		  std::cout << "Odd length in OKI/ADPCM not supported" << '\n';
	       }
             if((buf->size() % _nbChannels) != 0)
               {
                  std::cout << "OKI/ADPCM should read a multiple of channel number" << '\n';
               }
	     
             size_t count = 0;
             const size_t nbElems = buf->size();
             auto& out = *buf;
             
             while(count < nbElems)
               {
                  if(_currentChannel == 0 && !_decoders[0].hasNextSampleInCurrentBlock())
                    {
                       if(_src->isFinished())
                         {
                            out[count++] = 0;
                            continue;
                         }
                       
                       size_t minRequiredSize = nbElems - count;
                       const size_t roundNumber = 4UL * _nbChannels;
                       minRequiredSize = ((minRequiredSize + roundNumber - 1) / roundNumber) * roundNumber;
                       ensureTempBufferSize(minRequiredSize);
                       
                       _src->read(&_encodedBuffer);
                       for(auto& decoder : _decoders)
                         {
                            decoder.resetBuffer(_encodedBuffer);
                         }
                    }
                  
                  const s16 sample = _decoders[_currentChannel].getSample();
                  out[count++] = static_cast<s16>(static_cast<u16>(sample) << 4U);
                  ++_currentChannel;
                  if(_currentChannel == _decoders.size())
                    {
                       _currentChannel = 0;
                    }
               }
	  }
	
	auto OKIADPCMDecoderConverter::allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>
	  {
	     _encodedVec.resize(len);
	     u8* ptr = _encodedVec.data();
	     auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
	     auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
	     return mods::utils::RWBuffer<u8>(std::move(buffer));
	  }
        
        void OKIADPCMDecoderConverter::ensureTempBufferSize(size_t len)
          {
             if(_encodedBuffer.size() < len)
               {
                  _encodedBuffer = allocateNewTempBuffer(len);
               }
          }
        
        OKIADPCMDecoderConverter::Decoder::Decoder(int numChannel,
                                                   mods::utils::RBuffer<u8> encodedBuffer,
                                                   int nbChannels)
          : _encodedBuffer(std::move(encodedBuffer)),
          _itEncodedBuffer(_encodedBuffer.end()),
          _nbChannels(nbChannels),
          _numChannel(numChannel),
          _stepSize(mods::utils::at(stepSizeTable, _index))
          {
          }
        
        void OKIADPCMDecoderConverter::Decoder::resetBuffer(const mods::utils::RBuffer<u8>& encodedBuffer)
          {
             _sampleAvailable = false;
             _encodedBuffer = encodedBuffer;
             _itEncodedBuffer = _encodedBuffer.begin() + static_cast<ssize_t>(4L * _numChannel);
             _currentByteInDataWord = 0;
          }
        
        auto OKIADPCMDecoderConverter::Decoder::hasNextSampleInCurrentBlock() const -> bool
          {
             if(_sampleAvailable)
               {
                  return true;
               }
             if(_itEncodedBuffer < _encodedBuffer.end())
               {
                  return true;
               }
             return false;
          }
        
        auto OKIADPCMDecoderConverter::Decoder::getSample() -> s16
          {
             if(_sampleAvailable)
               {
                  _sampleAvailable = false;
                  return _nextSample;
               }
             if(_itEncodedBuffer >= _encodedBuffer.RBuffer<u8>::end())
               {
                  return 0;
               }
             
             static constexpr u8 nibbleMask = 0xFU;
             
             const u8 encodedValue = *_itEncodedBuffer;
             int sample = static_cast<u8>(encodedValue >> 4U) & nibbleMask;
             const s16 decodedSample = decodeSample(sample);
             
             sample = encodedValue & nibbleMask;
             _nextSample = decodeSample(sample);
             _sampleAvailable = true;
             
             _currentByteInDataWord++;
             if(_currentByteInDataWord == 4)
               {
                  _currentByteInDataWord = 0;
                  _itEncodedBuffer += 1 + ((_nbChannels-1) * 4);
               }
             else
               {
                  ++_itEncodedBuffer;
               }
             
             return decodedSample;
          }
	
	auto OKIADPCMDecoderConverter::Decoder::decodeSample(int sample) -> s16
	  {
	     using mods::utils::at;
             
             static constexpr u32 signBitMask = 8U;
	     
	     const u32 originalSample = static_cast<u32>(sample);
	     int difference = 0;
	     if((originalSample & 4U) != 0)
	       {
		  difference += static_cast<int>(_stepSize);
	       }
	     if((originalSample & 2U) != 0)
	       {
		  difference += static_cast<int>(_stepSize >> 1U);
	       }
	     if((originalSample & 1U) != 0)
	       {
		  difference += static_cast<int>(_stepSize >> 2U);
	       }
	     difference += static_cast<int>(_stepSize >> 3U);
	     if((originalSample & signBitMask) != 0)
	       {
		  difference = -difference;
	       }
	     _newSample += difference;
             
             static constexpr int minSample = -2048;
             static constexpr int maxSample = 2047;
             
             _newSample = mods::utils::clamp(_newSample, minSample , maxSample);
	     
	     _index += at(indexTable, originalSample);
	     _index = mods::utils::clamp(_index, 0, static_cast<int>(stepSizeTable.size()-1));
	     _stepSize = at(stepSizeTable, _index);
	     
	     return static_cast<s16>(_newSample);
	  }
     } // namespace wav
} // namespace mods
