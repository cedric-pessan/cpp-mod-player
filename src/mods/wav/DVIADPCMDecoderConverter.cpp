
#include "mods/wav/DVIADPCMDecoderConverter.hpp"

#include <iostream>
#include <limits>

namespace mods
{
   namespace wav
     {
	DVIADPCMDecoderConverter::DVIADPCMDecoderConverter(WavConverter::ptr src, const Format& format)
	  : _src(std::move(src)),
          _blockSize(format.getBitsPerContainer() / BITS_IN_BYTE),
	  _encodedBuffer(allocateNewTempBuffer(_blockSize)),
          _dataBuffer(_encodedBuffer.slice<u8>(sizeof(impl::DVIADPCMHeader), _blockSize - sizeof(impl::DVIADPCMHeader))),
          _itDataBuffer(_dataBuffer.RBuffer<u8>::end()),
          _header(_encodedBuffer.slice<impl::DVIADPCMHeader>(0, sizeof(impl::DVIADPCMHeader)))
	    {
               if(_blockSize <= sizeof(impl::DVIADPCMHeader))
                 {
                    std::cout << "Warning: block size is too small" << std::endl;
                 }
               
               if(format.getNumChannels() != 1)
                 {
                    std::cout << "Warning: DVIADPCMDecoverConverter: multi channel stream" << std::endl;
                 }
               
               if(format.getBitsPerSample() != 4)
                 {
                    std::cout << "Warning: DVIADPCMDecoderConverter: stream is not 4 bits per sample" << std::endl;
                 }
               
               if(!format.hasMetaData())
                 {
                    std::cout << "Warning: DVIADPCMDecoderConverter: wav does not have required format extension" << std::endl;
                 }
               
               // TODO: check align...
               
               auto metadata = format.getMetaData();
               auto dviMetadata = metadata.slice<u16>(0,1);
               u16 samplesPerBlock = dviMetadata[0];
               
               if(((samplesPerBlock-1)*4 + sizeof(impl::DVIADPCMHeader) * BITS_IN_BYTE) != format.getBitsPerContainer())
                 {
                    std::cout << "Warning: DVIADPCMDecoderConverter: number of samples per block does not match bits per container" << std::endl;
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
	     return !_sampleAvailable && _itDataBuffer == _dataBuffer.RBuffer<u8>::end() && _src->isFinished();
	  }
	
	void DVIADPCMDecoderConverter::read(mods::utils::RWBuffer<u8>* buf, size_t len)
	  {
	     if((len & 1U) != 0)
	       {
		  std::cout << "Odd length in DVI/ADPCM not supported" << std::endl;
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
                            _itDataBuffer = _dataBuffer.RBuffer<u8>::begin();
                            out[count++] = _header->getFirstSample();
                            _newSample = _header->getFirstSample();
                            _index = _header->getStepSizeTableIndex();
                            _index = mods::utils::clamp(_index, 0, static_cast<int>(stepSizeTable.size()-1));
                         }
                       else
                         {
                            out[count++] = 0;
                         }
                    }
                  else
                    {
                       u8 v = *_itDataBuffer;
                       int sample = v & nibbleMask;
                       _nextSample = decodeSample(sample);
                       out[count++] = _nextSample;
		  
                       sample = static_cast<u8>(v >> 4U) & nibbleMask;
		       _nextSample = decodeSample(sample);
                       _sampleAvailable = true;
                       
                       ++_itDataBuffer;
                    }
               }
	  }
	
	auto DVIADPCMDecoderConverter::allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>
	  {
	     _encodedVec.resize(len);
	     u8* ptr = _encodedVec.data();
	     auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
	     auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
	     return mods::utils::RWBuffer<u8>(buffer);
	  }
	
	auto DVIADPCMDecoderConverter::decodeSample(int sample) -> s16
	  {
	     using mods::utils::at;
             
             static constexpr u32 signBitMask = 8U;
	     
	     u32 originalSample = static_cast<u32>(sample);
	     int difference = 0;
	     if((originalSample & 4U) != 0)
	       {
		  difference += _stepSize;
	       }
	     if((originalSample & 2U) != 0)
	       {
		  difference += (_stepSize >> 1U);
	       }
	     if((originalSample & 1U) != 0)
	       {
		  difference += (_stepSize >> 2U);
	       }
	     difference += (_stepSize >> 3U);
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
