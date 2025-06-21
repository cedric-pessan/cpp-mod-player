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
             struct Parameters
               {
                  int x;
                  int y;
                  u16 period;
               };
             
             Arpeggio();
             Arpeggio(const Arpeggio&) = delete;
             Arpeggio(Arpeggio&&) = delete;
             auto operator=(const Arpeggio&) -> Arpeggio& = delete;
             auto operator=(Arpeggio&&) -> Arpeggio& = delete;
             ~Arpeggio() override = default;
             
             void init(Parameters parameters);
             
             auto getModifiedPeriod(u16 period) -> u16 override;
             auto getModifiedVolume(u16 volume) const -> u16 override;
             void tick() override;
             
           private:
             int _currentNote = 0;
             
             constexpr static int ARPEGGIO_FACTOR_TABLE_SIZE = 16;
             std::array<double, ARPEGGIO_FACTOR_TABLE_SIZE> _fineTuneFactors {};
             
             int _x = 0;
             int _y = 0;
             u16 _period = 0;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_ARPEGGIO_HPP
