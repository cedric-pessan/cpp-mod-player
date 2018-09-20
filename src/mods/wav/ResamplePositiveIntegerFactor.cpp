
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
     };
} // namespace mods
