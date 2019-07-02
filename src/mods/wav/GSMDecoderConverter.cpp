
#include "mods/wav/GSMDecoderConverter.hpp"

namespace mods
{
   namespace wav
     {
        GSMDecoderConverter::GSMDecoderConverter(WavConverter::ptr src)
          : _src(std::move(src))
            {
            }
        
        bool GSMDecoderConverter::isFinished() const
          {
             std::cout << "TODO: GSMDecoderConverter::isFinished() const" << std::endl;
             return false;
          }
        
        void GSMDecoderConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             _src->read(buf, len);
             std::cout << "TODO: GSMDecoderConverter::read(mods::utils::RWBuffer<u8>*, int)" << std::endl;
          }
     } // namespace wav
} // namespace mods