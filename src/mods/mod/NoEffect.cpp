
#include "mods/mod/NoEffect.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace mod
     {
        auto NoEffect::getModifiedPeriod(u16 period) -> u16
          {
             return period;
          }
        
        auto NoEffect::getModifiedVolume(u16 volume) const -> u16
          {
             return volume;
          }
        
        void NoEffect::tick()
          {
          }
     } // namespace mod
} // namespace mods
