
#include "mods/mod/SlideUp.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace mod
     {
        void SlideUp::init(int delta)
          {
             _newTick = false;
             _delta = delta;
          }
        
        auto SlideUp::getModifiedPeriod(u16 period) -> u16
          {
             if(_newTick)
               {
                  _newTick = false;
                  return period - _delta;
               }
             return period;
          }
        
        auto SlideUp::getModifiedVolume(u16 volume) const -> u16
          {
             return volume;
          }
        
        auto SlideUp::retriggerSample() const -> bool
          {
             return false;
          }
        
        void SlideUp::tick()
          {
             _newTick = true;
          }
     } // namespace mod
} // namespace mods
