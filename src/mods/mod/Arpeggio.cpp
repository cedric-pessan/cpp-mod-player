
#include "mods/mod/Arpeggio.hpp"

#include <cmath>
#include <iostream>

namespace mods
{
   namespace mod
     {
        Arpeggio::Arpeggio()
          {
             using mods::utils::at;
             
             double halfToneFactor = std::pow(2.0, -1.0 / (12.0 * 8.0));
             
             for(int i=0; i<16; ++i)
               {
                  auto fineTune = static_cast<double>(i);
                  double factor = std::pow(halfToneFactor, fineTune);
                  at(_fineTuneFactors, i) = factor;
               }
          }
        
        void Arpeggio::init(int x, int y, u16 period)
          {
             _currentNote = 0;
             _x = x;
             _y = y;
             _period = period;
          }
        
        auto Arpeggio::getModifiedPeriod(u16 period) -> u16
          {
             using mods::utils::at;
             
             if(_currentNote == 0)
               {
                  return _period;
               }
             else
               {
                  auto semitones = _y;
                  if(_currentNote == 1)
                    {
                       semitones = _x;
                    }
                  auto factor = at(_fineTuneFactors, semitones);
                  return std::round(static_cast<double>(_period) * factor);
               }
          }
        
        auto Arpeggio::getModifiedVolume(u16 volume) const -> u16
          {
             return volume;
          }
        
        void Arpeggio::tick()
          {
             _currentNote = (_currentNote + 1) % 3;
          }
     } // namespace mod
} // namespace mods
