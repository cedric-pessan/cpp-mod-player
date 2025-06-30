
#include "mods/mod/SlideToNote.hpp"

#include <algorithm>
#include <iostream>

namespace mods
{
   namespace mod
     {
        void SlideToNote::init(u16 targetPeriod, u16 speed)
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
                       u16 delta = std::min(_speed, static_cast<u16>(period - _targetPeriod));
                       return period - delta;
                    }
                  else if(period < _targetPeriod)
                    {
                       std::cout << "TODO: SlideToNote::getModifiedPeriod(u16) <" << std::endl;
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

