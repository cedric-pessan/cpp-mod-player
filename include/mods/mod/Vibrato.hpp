#ifndef MODS_MOD_VIBRATO_HPP
#define MODS_MOD_VIBRATO_HPP

#include "mods/mod/Effect.hpp"

#include <array>

namespace mods
{
   namespace mod
     {
        class Vibrato : public Effect
          {
           public:
             enum Depth : int;
             enum VibratoFrequency : int;
             
             Vibrato();
             Vibrato(const Vibrato&) = delete;
             Vibrato(Vibrato&&) = default;
             auto operator=(const Vibrato&) -> Vibrato& = delete;
             auto operator=(Vibrato&&) -> Vibrato& = delete;
             ~Vibrato() override = default;
             
             void init(Depth depth, VibratoFrequency oscillationFrequency, u16 period);
             
             auto getModifiedPeriod(u16 period) -> u16 override;
             auto getModifiedVolume(u16 volume) const -> u16 override;
             auto retriggerSample() const -> bool override;
             void tick() override;
             
           private:
             int _depth = 0;
             int _oscillationFrequency = 0;
             int _sinePos = 0;
             bool _negSine = false;
             u16 _period = 0;
             
             constexpr static int SINE_TABLE_SIZE = 32;
             std::array<int, SINE_TABLE_SIZE> _sine {};
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_VIBRATO_HPP
