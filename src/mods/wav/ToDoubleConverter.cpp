
#include "mods/wav/ToDoubleConverter.hpp"

namespace mods
{
   namespace wav
     {
        ToDoubleConverter::ToDoubleConverter(WavConverter::ptr src)
          : _src(std::move(src))
            {
            }
        
        bool ToDoubleConverter::isFinished() const
          {
             return _src->isFinished();
          }
        
        void ToDoubleConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             _src->read(buf, len);
             std::cout << "TODO: ToDoubleConverter::read(mods::utils::RWBuffer<u8>*, int)" << std::endl;
          }
     } // namespace wav
} // namespace mods
