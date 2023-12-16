#ifndef MODS_MOD_EFFECT_HPP
#define MODS_MOD_EFFECT_HPP

#include "mods/utils/types.hpp"

namespace mods
{
   namespace mod
     {
        class Effect
          {
           public:
             Effect() = default;
             Effect(const Effect&) = delete;
             Effect(Effect&&) = default;
             auto operator=(const Effect&) -> Effect& = delete;
             auto operator=(Effect&&) -> Effect& = delete;
             virtual ~Effect() = default;
             
             virtual auto getModifiedPeriod(u16 period) -> u16 = 0;
             virtual auto getModifiedVolume(u16 volume) const -> u16 = 0;
             virtual void tick() = 0;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_EFFECT_HPP
