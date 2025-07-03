#ifndef MODS_MOD_SLIDETONOTE_HPP
#define MODS_MOD_SLIDETONOTE_HPP

#include "mods/mod/Effect.hpp"

namespace mods
{
   namespace mod
     {
        class SlideToNote : public Effect
          {
           public:
             enum Period : u16;
             
             SlideToNote() = default;
             SlideToNote(const SlideToNote&) = delete;
             SlideToNote(SlideToNote&&) = delete;
             auto operator=(const SlideToNote&) -> SlideToNote& = delete;
             auto operator=(SlideToNote&&) -> SlideToNote& = delete;
             ~SlideToNote() override = default;
             
             void init(Period targetPeriod, u16 speed);
             
             auto getModifiedPeriod(u16 period) -> u16 override;
             auto getModifiedVolume(u16 volume) const -> u16 override;
             void tick() override;
             
           private:
             bool _newTick = false;
             u16 _targetPeriod = 0;
             u16 _speed = 0;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_SLIDETONOTE_HPP