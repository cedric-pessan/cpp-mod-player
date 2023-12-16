#ifndef MODS_MOD_ARPEGGIO_HPP
#define MODS_MOD_ARPEGGIO_HPP

#include "mods/mod/Effect.hpp"

#include <array>

namespace mods
{
   namespace mod
     {
        class Arpeggio : public Effect
          {
           public:
             Arpeggio();
             Arpeggio(const Arpeggio&) = delete;
             Arpeggio(Arpeggio&&) = delete;
             auto operator=(const Arpeggio&) -> Arpeggio& = delete;
             auto operator=(Arpeggio&&) -> Arpeggio& = delete;
             ~Arpeggio() override = default;
             
             void init(int x, int y, u16 period);
             
             auto getModifiedPeriod(u16 period) -> u16 override;
             auto getModifiedVolume(u16 volume) const -> u16 override;
             void tick() override;
             
           private:
             int _currentNote = 0;
             std::array<double, 16> _fineTuneFactors;
             
             int _x = 0;
             int _y = 0;
             u16 _period;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_ARPEGGIO_HPP
