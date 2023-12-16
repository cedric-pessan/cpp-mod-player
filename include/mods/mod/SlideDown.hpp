#ifndef MODS_MOD_SLIDEDOWN_HPP
#define MODS_MOD_SLIDEDOWN_HPP

#include "mods/mod/Effect.hpp"

namespace mods
{
   namespace mod
     {
        class SlideDown : public Effect
          {
           public:
             SlideDown() = default;
             SlideDown(const SlideDown&) = delete;
             SlideDown(SlideDown&&) = delete;
             auto operator=(const SlideDown&) -> SlideDown& = delete;
             auto operator=(SlideDown&&) -> SlideDown& = delete;
             ~SlideDown() override = default;
             
             void init(int increment);
             
             auto getModifiedPeriod(u16 period) -> u16 override;
             auto getModifiedVolume(u16 volume) const -> u16 override;
             void tick() override;
             
           private:
             bool _newTick = false;
             int _increment = 0;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_SLIDEDOWN_HPP