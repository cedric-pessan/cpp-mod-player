
#include "mods/mod/SlideDown.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace mod
     {
        void SlideDown::init(int delta)
          {
             _newTick = false;
             _delta = delta;
          }
        
        auto SlideDown::getModifiedPeriod(u16 period) -> u16
          {
             if(_newTick)
               {
                  _newTick = false;
                  return period + _delta;
               }
             return period;
          }
        
        auto SlideDown::getModifiedVolume(u16 volume) const -> u16
          {
             return volume;
          }
        
        void SlideDown::tick()
          {
             _newTick = true;
          }
     } // namespace mod
} // namespace mods
