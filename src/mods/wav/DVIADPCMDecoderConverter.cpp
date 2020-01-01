
#include "mods/wav/DVIADPCMDecoderConverter.hpp"

namespace mods
{
   namespace wav
     {
	DVIADPCMDecoderConverter::DVIADPCMDecoderConverter(WavConverter::ptr src)
	  : _src(std::move(src))
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
	     if(_sampleAvailable) --samplesToRead;
	     int bytesToRead = samplesToRead / 2;
	     auto inbuf = buf->slice<u8>(len - bytesToRead, bytesToRead);
	     
	     _src->read(&inbuf, bytesToRead);
	     
	     /*for(int i=0; i<len/2; ++i)
	       {
		  int v = inbuf[i];
		  int sample = (v >> 4) & 0xF;
		  
		  buf[i*2] = newSample;
		  
		  sample = v & 0xF;
		  buf[i*2+1] = newSample;
	       }*/
	     
	     std::cout << "TODO: DVIADPCMDecoderConverter::read(mods::utils::RWBuffer<u8>* buf, int len)" << std::endl;
	  }
     } // namespace wav
} // namespace mods
