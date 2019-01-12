
#include "mods/wav/FromDoubleConverter.hpp"

namespace mods
{
   namespace wav
     {
        FromDoubleConverter::FromDoubleConverter(WavConverter::ptr src)
          : _src(std::move(src))
            {
            }
        
        bool FromDoubleConverter::isFinished() const
          {
             return _src->isFinished();
          }
        
        void FromDoubleConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             _src->read(buf, len);
             std::cout << "TODO: FromDoubleConverter::read(mods::utils::RWBuffer<u8>*, int)" << std::endl;
          }
     } // namespace wav
} // namespace mods
