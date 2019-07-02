
#include "mods/wav/ShiftLeftConverter.hpp"

namespace mods
{
   namespace wav
     {
        ShiftLeftConverter::ShiftLeftConverter(WavConverter::ptr src)
          : _src(std::move(src))
            {
            }
        
        bool ShiftLeftConverter::isFinished() const
          {
             std::cout << "TODO: ShiftLeftConverter::isFinished() const" << std::endl;
             return false;
          }
        
        void ShiftLeftConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             _src->read(buf, len);
             std::cout << "TODO: ShiftLeftConverter::read(mods::utils::RWBuffer<u8>*, int)" << std::endl;
          }
     } // namespace wav
} // namespace mods