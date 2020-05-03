
#include "mods/wav/DVIADPCMDecoderConverter.hpp"

#include <iostream>
#include <limits>

namespace mods
{
   namespace wav
     {
	DVIADPCMDecoderConverter::DVIADPCMDecoderConverter(WavConverter::ptr src)
	  : _src(std::move(src)),
	  _temp(allocateNewTempBuffer(0))
	    {
	    }
	
	auto DVIADPCMDecoderConverter::isFinished() const -> bool
	  {
	     return !_sampleAvailable && _src->isFinished();
	  }
	
	void DVIADPCMDecoderConverter::read(mods::utils::RWBuffer<u8>* buf, size_t len)
	  {
	     if((len & 1U) != 0)
	       {
		  std::cout << "Odd length in DVI/ADPCM not supported" << std::endl;
	       }
	     
	     size_t samplesToRead = len / 2;
	     if(_sampleAvailable)
	       {
		  --samplesToRead;
	       }
	     if((samplesToRead & 1U) != 0) 
	       {
		  ++samplesToRead;
	       }
	     size_t bytesToRead = samplesToRead / 2;
	     
	     ensureTempBufferSize(bytesToRead);
	     auto inView = _temp.slice<u8>(0, bytesToRead);
	     
	     size_t samplesToWrite = len / 2;
	     auto outView = buf->slice<s16>(0, samplesToWrite);
	     
	     _src->read(&inView, bytesToRead);
	     
	     size_t samplesWritten = 0;
	     
	     if(_sampleAvailable)
	       {
		  s16 newSample = decodeSample(_sample);
		  outView[samplesWritten++] = newSample;
		  _sampleAvailable = false;
	       }
             
             static constexpr u8 nibbleMask = 0xFU;
	     
	     for(size_t i=0; i<bytesToRead; ++i)
	       {
		  u8 v = inView[i];
		  _sample = static_cast<u8>(v >> 4U) & nibbleMask;
		  s16 newSample = decodeSample(_sample);
		  outView[samplesWritten++] = newSample;
		  
		  _sample = v & nibbleMask;
		  if(samplesWritten < samplesToWrite)
		    {
		       newSample = decodeSample(_sample);
		       outView[samplesWritten++] = newSample;
		    }
		  else
		    {
		       _sampleAvailable = true;
		    }
	       }
	  }
	
	auto DVIADPCMDecoderConverter::allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>
	  {
	     _tempVec.resize(len);
	     u8* ptr = _tempVec.data();
	     auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
	     auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
	     return mods::utils::RWBuffer<u8>(buffer);
	  }
        
	void DVIADPCMDecoderConverter::ensureTempBufferSize(size_t len)
	  {
	     if(_temp.size() < len)
	       {
		  _temp = allocateNewTempBuffer(len);
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
