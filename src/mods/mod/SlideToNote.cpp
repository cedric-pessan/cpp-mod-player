
#include "mods/mod/SlideToNote.hpp"
#include "mods/utils/types.hpp"

#include <algorithm>
#include <iostream>

namespace mods
{
   namespace mod
     {
        void SlideToNote::init(Period targetPeriod, u16 speed)
          {
             _newTick = false;
             if(targetPeriod != 0)
               {
                  _targetPeriod = targetPeriod;
               }
             if(speed != 0)
               {
                  _speed = speed;
               }
          }
        
        auto SlideToNote::getModifiedPeriod(u16 period) -> u16
          {
             if(_newTick)
               {
                  _newTick = false;
                  if(period > _targetPeriod)
                    {
                       const u16 delta = std::min(_speed, static_cast<u16>(period - _targetPeriod));
                       return period - delta;
                    }
                  if(period < _targetPeriod)
                    {
                       const u16 delta = std::min(_speed, static_cast<u16>(_targetPeriod - period));
                       return period + delta;
                    }
               }
             return period;
          }
        
        auto SlideToNote::getModifiedVolume(u16 volume) const -> u16
          {
             return volume;
          }
        
        void SlideToNote::tick()
          {
             _newTick = true;
          }
     } // namespace mod
} // namespace mods

