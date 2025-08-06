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
             
             void init(int retriggerPeriod);
             
             auto getModifiedPeriod(u16 period) -> u16 override;
             auto getModifiedVolume(u16 volume) const -> u16 override;
             auto retriggerSample() const -> bool override;
             void tick() override;
             
           private:
             int _tickCount = 0;
             int _retriggerPeriod = 0;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_RETRIGGER_HPP
