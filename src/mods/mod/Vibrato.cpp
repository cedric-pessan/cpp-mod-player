
#include "mods/mod/Vibrato.hpp"

#include <cmath>
#include <iostream>

namespace mods
{
   namespace mod
     {
        Vibrato::Vibrato()
          {
             using mods::utils::at;
             
             for(int i=0; i<32; ++i)
               {
                  double f = M_PI / 32.0 * (double)i;
                  int v = std::sin(f) * 255.0;
                  at(_sine, i) = v;
               }
          }
        
        void Vibrato::init(int depth, int oscillationFrequency, u16 period)
          {
             _depth = depth;
             _oscillationFrequency = oscillationFrequency;
             _sinePos = 0;
             _negSine = false;
             _period = period;
          }
        
        auto Vibrato::getModifiedPeriod(u16 period) -> u16
          {
             using mods::utils::at;
             
             int variation = _depth * at(_sine, _sinePos) / 128;
             if(_negSine)
               {
                  variation = -variation;
               }
             
             return _period + variation;
          }
        
        auto Vibrato::getModifiedVolume(u16 volume) const -> u16
          {
             return volume;
          }
        
        void Vibrato::tick()
          {
             _sinePos += _oscillationFrequency;
             
             while(_sinePos > 31)
               {
                  _sinePos -= 32;
                  _negSine = !_negSine;
               }
          }
     } // namespace mod
} // namespace mods
