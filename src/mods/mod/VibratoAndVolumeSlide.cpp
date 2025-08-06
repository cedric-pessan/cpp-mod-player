
#include "mods/mod/Vibrato.hpp"
#include "mods/mod/VibratoAndVolumeSlide.hpp"
#include "mods/mod/VolumeSlide.hpp"
#include "mods/utils/types.hpp"

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
        
        void VibratoAndVolumeSlide::init(u16 currentVolume, s16 delta)
          {
             using Volume = VolumeSlide::Volume;
             using Delta = VolumeSlide::Delta;
             
             _volumeSlide->init(static_cast<Volume>(currentVolume), static_cast<Delta>(delta));
          }
        
        auto VibratoAndVolumeSlide::getModifiedPeriod(u16 period) -> u16
          {
             return _vibrato->getModifiedPeriod(period);
          }
        
        auto VibratoAndVolumeSlide::getModifiedVolume(u16 volume) const -> u16
          {
             return _volumeSlide->getModifiedVolume(volume);
          }
        
        auto VibratoAndVolumeSlide::retriggerSample() const -> bool
          {
             return false;
          }
        
        void VibratoAndVolumeSlide::tick()
          {
             _vibrato->tick();
             _volumeSlide->tick();
          }
     } // namespace mod
} // namespace mods
