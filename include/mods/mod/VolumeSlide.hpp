#ifndef MODS_MOD_VOLUMESLIDE_HPP
#define MODS_MOD_VOLUMESLIDE_HPP

#include "mods/mod/Effect.hpp"

namespace mods
{
   namespace mod
     {
        class VolumeSlide : public Effect
          {
           public:
             VolumeSlide() = default;
             VolumeSlide(const VolumeSlide&) = delete;
             VolumeSlide(VolumeSlide&&) = delete;
             auto operator=(const VolumeSlide&) -> VolumeSlide& = delete;
             auto operator=(VolumeSlide&&) -> VolumeSlide& = delete;
             ~VolumeSlide() = default;
             
             void init(u16 currentVolume, u8 slideUp, u8 slideDown);
             
             auto getModifiedPeriod(u16 period) -> u16 override;
             auto getModifiedVolume(u16 volume) const -> u16 override;
             void tick();
             
           private:
             u16 _volume = 0;
             s16 _delta = 0;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_VOLUMESLIDE_HPP
