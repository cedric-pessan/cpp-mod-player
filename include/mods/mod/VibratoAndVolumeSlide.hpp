#ifndef MODS_MOD_VIBRATOANDVOLUMESLIDE_HPP
#define MODS_MOD_VIBRATOANDVOLUMESLIDE_HPP

#include "mods/mod/Effect.hpp"

namespace mods
{
   namespace mod
     {
        class Vibrato;
        class VolumeSlide;
        
        class VibratoAndVolumeSlide : public Effect
          {
           public:
             VibratoAndVolumeSlide(Vibrato* vibrato,
                                   VolumeSlide* volumeSlide);
             
             VibratoAndVolumeSlide() = delete;
             VibratoAndVolumeSlide(const VibratoAndVolumeSlide&) = delete;
             VibratoAndVolumeSlide(VibratoAndVolumeSlide&&) = delete;
             auto operator=(const VibratoAndVolumeSlide&) -> VibratoAndVolumeSlide& = delete;
             auto operator=(VibratoAndVolumeSlide&&) -> VibratoAndVolumeSlide& = delete;
             ~VibratoAndVolumeSlide() override = default;
             
             void init(u16 currentVolume, s16 delta);
             
             auto getModifiedPeriod(u16 period) -> u16 override;
             auto getModifiedVolume(u16 volume) const -> u16 override;
             auto retriggerSample() const -> bool override;
             void tick() override;
             
           private:
             Vibrato* _vibrato;
             VolumeSlide* _volumeSlide;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_VIBRATOANDVOLUMESLIDE_HPP
