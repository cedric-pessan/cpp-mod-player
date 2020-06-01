
#include "mods/wav/ADPCMDecoderConverter.hpp"

#include <iostream>
/*#include <limits>*/

namespace mods
{
   namespace wav
     {
	ADPCMDecoderConverter::ADPCMDecoderConverter(WavConverter::ptr src, int bitsPerContainer)
	  : _src(std::move(src)),
          _decodedBuffer(getDecodedBufferSize(bitsPerContainer) > 0 ? getDecodedBufferSize(bitsPerContainer) : 0),
          _itDecodedBuffer(_decodedBuffer.end()),
          _encodedVec(bitsPerContainer),
          _encodedBuffer(initializeRWBuffer(_encodedVec))
	    {
               if(getDecodedBufferSize(bitsPerContainer) < 0)
                 {
                    std::cout << "Warning: bits per container is too small for adpcm" << std::endl;
                 }
	    }
        
        template<typename ARRAY>
          auto ADPCMDecoderConverter::initializeRWBuffer(ARRAY& backArray) -> mods::utils::RWBuffer<typename ARRAY::value_type>
          {
             auto* ptr = static_cast<u8*>(static_cast<void*>(backArray.data()));
             auto len = backArray.size();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, len * sizeof(typename ARRAY::value_type), std::move(deleter));
             return mods::utils::RWBuffer<u8>(buffer).slice<typename ARRAY::value_type>(0, len);
          }
	
	auto ADPCMDecoderConverter::isFinished() const -> bool
	  {
             return _itDecodedBuffer == _decodedBuffer.end() && _src->isFinished();
	  }
	
	void ADPCMDecoderConverter::read(mods::utils::RWBuffer<u8>* buf, size_t len)
	  {
             if((len & 1U) != 0)
	       {
		  std::cout << "Odd length in ADPCM not supported" << std::endl;
	       }
             
             size_t count = 0;
             size_t nbElems = len / 2;
             auto out = buf->slice<s16>(0, nbElems);
             
             while(count < nbElems)
               {
                  if(_itDecodedBuffer != _decodedBuffer.end())
                    {
                       out[count++] = *_itDecodedBuffer;
                       ++_itDecodedBuffer;
                    }
                  else
                    {
                       if(_src->isFinished())
                         {
                            out[count++] = 0;
                         }
                       else
                         {
                            decodeBlock();
                            _itDecodedBuffer = _decodedBuffer.begin();
                         }
                    }
               }
             
	     /*
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
	       }*/
	  }
	
        void ADPCMDecoderConverter::decodeBlock()
          {
             _src->read(&_encodedBuffer, _encodedBuffer.size());
             
             auto preamble = _encodedBuffer.slice<impl::ADPCMPreamble>(0,1);
             size_t idxOut = 0;
             _decodedBuffer[idxOut++] = preamble->getSample1();
             _decodedBuffer[idxOut++] = preamble->getSample2();
             
             for(size_t i=sizeof(impl::ADPCMPreamble); i<_encodedBuffer.size(); ++i)
               {
                  _decodedBuffer[idxOut++] = _encodedBuffer[i];
               }
             
             std::cout << "TODO: decodeBlock" << std::endl;
          }
	
	/*namespace
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
	  }*/
     } // namespace wav
} // namespace mods
