
#include "mods/mod/NoEffect.hpp"

#include <iostream>

namespace mods
{
   namespace mod
     {
        auto NoEffect::getModifiedPeriod(u16 period) const -> u16
          {
             return period;
          }
        
        void NoEffect::tick()
          {
          }
     } // namespace mod
} // namespace mods
