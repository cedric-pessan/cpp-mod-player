
#include "mods/mod/SlideDown.hpp"

namespace mods
{
   namespace mod
     {
        void SlideDown::init(int increment)
          {
             _newTick = true;
             _increment = increment;
          }
        
        auto SlideDown::getModifiedPeriod(u16 period) -> u16
          {
             if(_newTick)
               {
                  _newTick = false;
                  return period + _increment;
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
