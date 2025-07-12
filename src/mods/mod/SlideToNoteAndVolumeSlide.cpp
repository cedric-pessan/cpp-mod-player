
#include "mods/mod/SlideToNote.hpp"
#include "mods/mod/SlideToNoteAndVolumeSlide.hpp"
#include "mods/mod/VolumeSlide.hpp"

#include <iostream>

namespace mods
{
   namespace mod
     {
        SlideToNoteAndVolumeSlide::SlideToNoteAndVolumeSlide(SlideToNote* slideToNote, VolumeSlide* volumeSlide)
          : _slideToNote(slideToNote),
          _volumeSlide(volumeSlide)
            {
            }
        
        void SlideToNoteAndVolumeSlide::init(Volume currentVolume, Delta delta)
          {
             _slideToNote->init(static_cast<SlideToNote::Period>(0), 0); // keep previous parameers
             _volumeSlide->init(currentVolume, delta);
          }
        
        auto SlideToNoteAndVolumeSlide::getModifiedPeriod(u16 period) -> u16
          {
             return _slideToNote->getModifiedPeriod(period);
          }
        
        auto SlideToNoteAndVolumeSlide::getModifiedVolume(u16 volume) const -> u16
          {
             return _volumeSlide->getModifiedVolume(volume);
          }
        
        void SlideToNoteAndVolumeSlide::tick()
          {
             _slideToNote->tick();
             _volumeSlide->tick();
          }
     } // namespace mod
} // namespace mods
