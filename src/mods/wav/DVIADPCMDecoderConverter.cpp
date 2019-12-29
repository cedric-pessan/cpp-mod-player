
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
	     std::cout << "TODO: DVIADPCMDecoderConverter::isFinished() const" << std::endl;
	     return false;
	  }
	
	void DVIADPCMDecoderConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
	  {
	     _src->read(buf, len);
	     std::cout << "TODO: DVIADPCMDecoderConverter::read(mods::utils::RWBuffer<u8>* buf, int len)" << std::endl;
	  }
     } // namespace wav
} // namespace mods
