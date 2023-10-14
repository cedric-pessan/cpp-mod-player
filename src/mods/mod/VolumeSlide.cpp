
#include "mods/mod/VolumeSlide.hpp"

#include <iostream>

namespace mods
{
   namespace mod
     {
        void VolumeSlide::init(u16 currentVolume, u8 slideUp, u8 slideDown)
          {
             _volume = currentVolume;
             if(slideUp > 0 && slideDown > 0)
               {
                  _delta = 0;
               }
             else if(slideUp > 0)
               {
                  _delta = slideUp;
               }
             else
               {
                  _delta = -slideDown;
               }
          }
        
        auto VolumeSlide::getModifiedPeriod(u16 period) const -> u16
          {
             std::cout << "TODO: VolumeSlide::getModifiedPeriod(u16) const" << std::endl;
             return period;
          }
        
        auto VolumeSlide::getModifiedVolume(u16 volume) const -> u16
          {
             return _volume;
          }
        
        void VolumeSlide::tick()
          {
             s16 v = _volume;
             v += _delta;
             if(v > 64)
               {
                  v = 64;
               }
             else if(v < 0)
               {
                  v = 0;
               }
             _volume = static_cast<u16>(v);
          }
     } // namespace mod
} // namespace mods
