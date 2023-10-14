
#include "mods/mod/Vibrato.hpp"
#include "mods/mod/VibratoAndVolumeSlide.hpp"
#include "mods/mod/VolumeSlide.hpp"

#include <iostream>

namespace mods
{
   namespace mod
     {
        VibratoAndVolumeSlide::VibratoAndVolumeSlide(Vibrato* vibrato,
                                                     VolumeSlide* volumeSlide)
          : _vibrato(vibrato),
          _volumeSlide(volumeSlide)
            {
            }
        
        void VibratoAndVolumeSlide::init(u16 currentVolume, u8 slideUp, u8 slideDown)
          {
             _volumeSlide->init(currentVolume, slideUp, slideDown);
          }
        
        auto VibratoAndVolumeSlide::getModifiedPeriod(u16 period) const -> u16
          {
             return _vibrato->getModifiedPeriod(period);
          }
        
        auto VibratoAndVolumeSlide::getModifiedVolume(u16 volume) const -> u16
          {
             return _volumeSlide->getModifiedVolume(volume);
          }
        
        void VibratoAndVolumeSlide::tick()
          {
             _vibrato->tick();
             _volumeSlide->tick();
          }
     } // namespace mod
} // namespace mods
