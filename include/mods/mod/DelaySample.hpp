#ifndef MODS_MOD_DELAYSAMPLE_HPP
#define MODS_MOD_DELAYSAMPLE_HPP

#include "mods/mod/Effect.hpp"

namespace mods
{
   namespace mod
     {
        class DelaySample : public Effect
          {
           public:
             DelaySample() = default;
             DelaySample(const DelaySample&) = delete;
             DelaySample(DelaySample&&) = delete;
             auto operator=(const DelaySample&) -> DelaySample& = delete;
             auto operator=(DelaySample&&) -> DelaySample& = delete;
             ~DelaySample() override = default;
             
             void init(u32 delayCounter);
             
             auto getModifiedPeriod(u16 period) -> u16 override;
             auto getModifiedVolume(u16 volume) const -> u16 override;
             auto retriggerSample() const -> bool override;
             auto isSampleEnabled() const -> bool override;
             void tick() override;
             
           private:
             bool _retrigger = false;
             bool _sampleEnabled = false;
             int _delayCounter = 0;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_DELAYSAMPLE_HPP
