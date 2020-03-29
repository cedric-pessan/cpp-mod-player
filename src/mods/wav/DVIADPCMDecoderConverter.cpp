
#include "mods/wav/DVIADPCMDecoderConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
	DVIADPCMDecoderConverter::DVIADPCMDecoderConverter(WavConverter::ptr src)
	  : _src(std::move(src)),
	  _temp(allocateNewTempBuffer(0))
	    {
	    }
	
	bool DVIADPCMDecoderConverter::isFinished() const
	  {
	     return !_sampleAvailable && _src->isFinished();
	  }
	
	void DVIADPCMDecoderConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
	  {
	     if((len & 1) != 0)
	       {
		  std::cout << "Odd length in DVI/ADPCM not supported" << std::endl;
	       }
	     
	     int samplesToRead = len / 2;
	     if(_sampleAvailable)
	       {
		  --samplesToRead;
	       }
	     if((samplesToRead & 1) != 0) 
	       {
		  ++samplesToRead;
	       }
	     int bytesToRead = samplesToRead / 2;
	     
	     ensureTempBufferSize(bytesToRead);
	     auto inView = _temp.slice<u8>(0, bytesToRead);
	     
	     int samplesToWrite = len / 2;
	     auto outView = buf->slice<s16>(0, samplesToWrite);
	     
	     _src->read(&inView, bytesToRead);
	     
	     int samplesWritten = 0;
	     
	     if(_sampleAvailable)
	       {
		  s16 newSample = decodeSample(_sample);
		  outView[samplesWritten++] = newSample;
		  _sampleAvailable = false;
	       }
	     
	     for(int i=0; i<bytesToRead; ++i)
	       {
		  int v = inView[i];
		  _sample = (v >> 4) & 0xF;
		  s16 newSample = decodeSample(_sample);
		  outView[samplesWritten++] = newSample;
		  
		  _sample = v & 0xF;
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
	
	mods::utils::RWBuffer<u8> DVIADPCMDecoderConverter::allocateNewTempBuffer(size_t len)
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
	     std::array<int, 16> indexTable
	       {
		  -1, -1, -1, -1, 2, 4, 6, 8,
		    -1, -1, -1, -1, 2, 4, 6, 8
	       };
	     
	     std::array<int, 89> stepSizeTable
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
	
	s16 DVIADPCMDecoderConverter::decodeSample(int sample)
	  {
	     using mods::utils::at;
	     
	     u32 originalSample = static_cast<u32>(sample);
	     int difference = 0;
	     if(originalSample & 4)
	       {
		  difference += _stepSize;
	       }
	     if(originalSample & 2)
	       {
		  difference += (_stepSize >> 1);
	       }
	     if(originalSample & 1)
	       {
		  difference += (_stepSize >> 2);
	       }
	     difference += (_stepSize >> 3);
	     if(originalSample & 8)
	       {
		  difference = -difference;
	       }
	     _newSample += difference;
	     _newSample = mods::utils::clamp(_newSample, -32768, 32767);
	     
	     _index += at(indexTable, originalSample);
	     _index = mods::utils::clamp(_index, 0, 88);
	     _stepSize = at(stepSizeTable, _index);
	     
	     return static_cast<s16>(_newSample);
	  }
     } // namespace wav
} // namespace mods
