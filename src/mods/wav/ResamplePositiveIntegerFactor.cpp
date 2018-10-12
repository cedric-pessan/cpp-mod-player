
#include "mods/wav/ResamplePositiveIntegerFactor.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        ResamplePositiveIntegerFactor::ResamplePositiveIntegerFactor(WavConverter::ptr src)
          : _src(std::move(src))
            {
            }
        
        bool ResamplePositiveIntegerFactor::isFinished() const
          {
             return _src->isFinished();
          }
        
        void ResamplePositiveIntegerFactor::read(mods::utils::RWBuffer<u8>& buf, int len)
          {
             _src->read(buf, len);
             std::cout << "TODO: ResamplePositiveIntegerFactor::read(...)" << std::endl;
          }
     };
} // namespace mods
