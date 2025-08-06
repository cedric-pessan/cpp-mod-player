
#include "mods/mod/Arpeggio.hpp"
#include "mods/utils/types.hpp"

#include <cmath>
#include <cstddef>

namespace mods
{
   namespace mod
     {
        Arpeggio::Arpeggio()
          {
             using mods::utils::at;
             
             const double halfToneFactor = std::pow(2.0, -1.0 / (12.0 * 8.0));
             
             for(size_t i=0; i<_fineTuneFactors.size(); ++i)
               {
                  auto fineTune = static_cast<double>(i);
                  const double factor = std::pow(halfToneFactor, fineTune);
                  at(_fineTuneFactors, i) = factor;
               }
          }
        
        void Arpeggio::init(Parameters parameters)
          {
             _currentNote = 0;
             _x = parameters.x;
             _y = parameters.y;
             _period = parameters.period;
          }
        
        auto Arpeggio::getModifiedPeriod(u16 /* period */) -> u16
          {
             using mods::utils::at;
             
             if(_currentNote == 0)
               {
                  return _period;
               }
             
             auto semitones = _y;
             if(_currentNote == 1)
               {
                  semitones = _x;
               }
             auto factor = at(_fineTuneFactors, semitones);
             return static_cast<u16>(std::round(static_cast<double>(_period) * factor));
          }
        
        auto Arpeggio::getModifiedVolume(u16 volume) const -> u16
          {
             return volume;
          }
        
        auto Arpeggio::retriggerSample() const -> bool
          {
             return false;
          }
        
        void Arpeggio::tick()
          {
             _currentNote = (_currentNote + 1) % 3;
          }
     } // namespace mod
} // namespace mods
