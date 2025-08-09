#ifndef MODS_MOD_RETRIGGER_HPP
#define MODS_MOD_RETRIGGER_HPP

#include "mods/mod/Effect.hpp"

namespace mods
{
   namespace mod
     {
        class Retrigger : public Effect
          {
           public:
             Retrigger() = default;
             Retrigger(const Retrigger&) = delete;
             Retrigger(Retrigger&&) = delete;
             auto operator=(const Retrigger&) -> Retrigger& = delete;
             auto operator=(Retrigger&&) -> Retrigger& = delete;
             ~Retrigger() override = default;
             
             void init(u32 retriggerPeriod);
             
             auto getModifiedPeriod(u16 period) -> u16 override;
             auto getModifiedVolume(u16 volume) const -> u16 override;
             auto retriggerSample() const -> bool override;
             void tick() override;
             
           private:
             u32 _tickCount = 0;
             u32 _retriggerPeriod = 0;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_RETRIGGER_HPP
