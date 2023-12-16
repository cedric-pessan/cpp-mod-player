#ifndef MODS_MOD_NOEFFECT_HPP
#define MODS_MOD_NOEFFECT_HPP

#include "mods/mod/Effect.hpp"

namespace mods
{
   namespace mod
     {
        class NoEffect : public Effect
          {
           public:
             NoEffect() = default;
             NoEffect(const NoEffect&) = delete;
             NoEffect(NoEffect&&) = default;
             auto operator=(const NoEffect&) -> NoEffect& = delete;
             auto operator=(NoEffect&&) -> NoEffect& = delete;
             ~NoEffect() override = default;
             
             auto getModifiedPeriod(u16 period) -> u16 override;
             auto getModifiedVolume(u16 volume) const -> u16 override;
             void tick() override;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_NOEFFECT_HPP
