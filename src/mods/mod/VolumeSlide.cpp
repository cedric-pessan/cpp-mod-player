
#include "mods/mod/VolumeSlide.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace mod
     {
        void VolumeSlide::init(Volume currentVolume, Delta delta)
          {
             _volume = currentVolume;
             _delta = delta;
          }
        
        auto VolumeSlide::getModifiedPeriod(u16 period) -> u16
          {
             return period;
          }
        
        auto VolumeSlide::getModifiedVolume(u16 /*volume*/) const -> u16
          {
             return _volume;
          }
        
        auto VolumeSlide::retriggerSample() const -> bool
          {
             return false;
          }
        
        void VolumeSlide::tick()
          {
             const int maxVolume = 64;
             
             s32 tmpVolume = _volume;
             tmpVolume += _delta;
             if(tmpVolume > maxVolume)
               {
                  tmpVolume = maxVolume;
               }
             else if(tmpVolume < 0)
               {
                  tmpVolume = 0;
               }
             _volume = static_cast<u16>(tmpVolume);
          }
     } // namespace mod
} // namespace mods
