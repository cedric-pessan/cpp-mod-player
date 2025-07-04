
#include "mods/converters/Converter.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/RWBufferBackend.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/DVIADPCMDecoderConverter.hpp"
#include "mods/wav/Format.hpp"
#include "mods/wav/impl/DVIADPCMDecoderConverterImpl.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>

namespace mods
{
   namespace wav
     {
	DVIADPCMDecoderConverter::DVIADPCMDecoderConverter(Converter<u8>::ptr src, const Format& format)
	  : _src(std::move(src)),
          _blockSize(format.getBitsPerContainer() / BITS_IN_BYTE * format.getNumChannels()),
          _nbChannels(format.getNumChannels()),
	  _encodedBuffer(allocateNewTempBuffer(_blockSize)),
          _dataBuffer(_encodedBuffer.readOnlySlice<u8>(sizeof(impl::DVIADPCMHeader) * _nbChannels, _blockSize - (sizeof(impl::DVIADPCMHeader) * _nbChannels))),
          _headers(_encodedBuffer.readOnlySlice<impl::DVIADPCMHeader>(0, _nbChannels))
	    {
               if(_blockSize <= sizeof(impl::DVIADPCMHeader))
                 {
                    std::cout << "Warning: block size is too small" << '\n';
                 }
               
               if(format.getBitsPerSample() != 4)
                 {
                    std::cout << "Warning: DVIADPCMDecoderConverter: stream is not 4 bits per sample" << '\n';
                 }
               
               if(!format.hasMetaData())
                 {
                    std::cout << "Warning: DVIADPCMDecoderConverter: wav does not have required format extension" << '\n';
                 }
               
               auto metadata = format.getMetaData();
               if(metadata.size() != sizeof(u16))
                 {
                    std::cout << "Warning: DVIADPCMDecoderConverter: wrong extension size" << '\n';
                 }
               
               auto dviMetadata = metadata.slice<u16>(0,1);
               const u16 samplesPerBlock = dviMetadata[0];
               
               if((static_cast<u64>((samplesPerBlock-1)*4) + sizeof(impl::DVIADPCMHeader) * BITS_IN_BYTE) != format.getBitsPerContainer())
                 {
                    std::cout << "Warning: DVIADPCMDecoderConverter: number of samples per block does not match bits per container" << '\n';
                 }
               
               if((static_cast<u64>((samplesPerBlock - 1) * _nbChannels) % (2 * sizeof(u32))) !=  0)
                 {
                    std::cout << "Warning: DVIADPCMDecoderConverter: samples per block must be a multiple of 8" << '\n';
                 }
               
               for(int i=0; i<_nbChannels; ++i)
                 {
                    _decoders.emplace_back(i, _dataBuffer, _headers, _nbChannels);
                 }
	    }
        
        namespace
	  {
	     constexpr std::array<int, 16> indexTable
	       {
		  -1, -1, -1, -1, 2, 4, 6, 8,
		    -1, -1, -1, -1, 2, 4, 6, 8
	       };
	     
	     constexpr std::array<int, 89> stepSizeTable
	       {
		  7, 8, 9, 10, 11, 12, 13, 14,
		    16, 17, 19, 21, 23, 25, 28,
		    31, 34, 37, 41, 45, 50, 55,
		    60, 66, 73, 80, 88, 97, 107,
		    118, 130, 143, 157, 173, 190, 209,
		    230, 253, 279, 307, 337, 371, 408,
		    449, 494, 544, 598, 658, 724, 796,
		    876, 963, 1060, 1166, 1282, 1411, 1552,
		    1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026,
		    4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
		    9493, 10442, 11487, 12635, 13899, 15289, 16818,
		    18500, 20350, 22385, 24623, 27086, 29794, 32767
	       };
	  } // namespace
	
	auto DVIADPCMDecoderConverter::isFinished() const -> bool
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
	
	void DVIADPCMDecoderConverter::read(mods::utils::RWBuffer<s16>* buf)
	  {
             if((buf->size() % _nbChannels) != 0)
               {
                  std::cout << "DVI/ADPCM should read a multiple of channel number" << '\n';
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
                       
                       _src->read(&_encodedBuffer);
                       for(auto& decoder : _decoders)
                         {
                            decoder.resetBuffer();
                         }
                    }
                  
                  const s16 sample = _decoders[_currentChannel].getSample();
                  out[count++] = sample;
                  ++_currentChannel;
                  if(_currentChannel == _decoders.size())
                    {
                       _currentChannel = 0;
                    }
               }
	  }
	
	auto DVIADPCMDecoderConverter::allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>
	  {
	     _encodedVec.resize(len);
	     u8* ptr = _encodedVec.data();
	     auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
	     auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
	     return mods::utils::RWBuffer<u8>(std::move(buffer));
	  }
        
        DVIADPCMDecoderConverter::Decoder::Decoder(int numChannel,
                                                   mods::utils::RBuffer<u8> dataBuffer,
                                                   const mods::utils::RBuffer<impl::DVIADPCMHeader>& headers,
                                                   int nbChannels)
          : _dataBuffer(std::move(dataBuffer)),
          _itDataBuffer(_dataBuffer.end()),
          _header(&headers[numChannel]),
          _nbChannels(nbChannels),
          _numChannel(numChannel)
          {
          }
        
        void DVIADPCMDecoderConverter::Decoder::resetBuffer()
          {
             _sampleAvailable = false;
             _itDataBuffer = _dataBuffer.begin() + 4L * _numChannel;
             _currentByteInDataWord = 0;
             _firstSampleInBlock = true;
             _index = 0;
             _newSample = 0;
             _nextSample = 0;
          }
        
        auto DVIADPCMDecoderConverter::Decoder::hasNextSampleInCurrentBlock() const -> bool
          {
             if(_sampleAvailable)
               {
                  return true;
               }
             if(_itDataBuffer < _dataBuffer.end())
               {
                  return true;
               }
             return false;
          }
        
        auto DVIADPCMDecoderConverter::Decoder::getSample() -> s16
          {
             if(_sampleAvailable)
               {
                  _sampleAvailable = false;
                  return _nextSample;
               }
             if(_itDataBuffer >= _dataBuffer.RBuffer<u8>::end())
               {
                  return 0;
               }
             if(_firstSampleInBlock)
               {
                  using mods::utils::at;
                  
                  _index = _header->getStepSizeTableIndex();
                  _index = mods::utils::clamp(_index, 0, static_cast<int>(stepSizeTable.size()-1));
                  _stepSize = at(stepSizeTable, _index);
                  _firstSampleInBlock = false;
                  _newSample = _header->getFirstSample();
                  return _header->getFirstSample();
               }
             
             static constexpr u8 nibbleMask = 0xFU;
             
             const u8 currentByte = *_itDataBuffer;
             int sample = currentByte & nibbleMask;
             const s16 decodedSample = decodeSample(sample);
             
             sample = static_cast<u8>(currentByte >> 4U) & nibbleMask;
             _nextSample = decodeSample(sample);
             _sampleAvailable = true;
             
             _currentByteInDataWord++;
             if(_currentByteInDataWord == 4)
               {
                  _currentByteInDataWord = 0;
                  _itDataBuffer += 1 + ((_nbChannels-1) * 4);
               }
             else
               {
                  ++_itDataBuffer;
               }
             
             return decodedSample;
          }
	
	auto DVIADPCMDecoderConverter::Decoder::decodeSample(int sample) -> s16
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
	     _newSample = mods::utils::clamp(_newSample, 
                                             static_cast<int>(std::numeric_limits<s16>::min()),
                                             static_cast<int>(std::numeric_limits<s16>::max()));
	     
	     _index += at(indexTable, originalSample);
	     _index = mods::utils::clamp(_index, 0, static_cast<int>(stepSizeTable.size()-1));
	     _stepSize = at(stepSizeTable, _index);
	     
	     return static_cast<s16>(_newSample);
	  }
     } // namespace wav
} // namespace mods
