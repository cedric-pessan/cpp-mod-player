
#include "mods/mod/Vibrato.hpp"
#include "mods/utils/MathConstants.hpp"
#include "mods/utils/types.hpp"

#include <cmath>
#include <cstddef>

namespace mods
{
   namespace mod
     {
        Vibrato::Vibrato()
          {
             using mods::utils::at;
             
             for(size_t i=0; i<_sine.size(); ++i)
               {
                  const double sineValue = mods::utils::math::cPI / 32.0 * static_cast<double>(i);
                  const int value = static_cast<int>(std::sin(sineValue) * 255.0);
                  at(_sine, i) = value;
               }
          }
        
        void Vibrato::init(Depth depth, VibratoFrequency oscillationFrequency, u16 period)
          {
             _depth = depth;
             _oscillationFrequency = oscillationFrequency;
             _sinePos = 0;
             _negSine = false;
             _period = period;
          }
        
        auto Vibrato::getModifiedPeriod(u16 /* period */) -> u16
          {
             using mods::utils::at;
             
             constexpr int vibratoDepthFactor = 128;
             
             int variation = _depth * at(_sine, _sinePos) / vibratoDepthFactor;
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
        
        auto Vibrato::retriggerSample() const -> bool
          {
             return false;
          }
        
        auto Vibrato::isSampleEnabled() const -> bool
          {
             return true;
          }
        
        void Vibrato::tick()
          {
             _sinePos += _oscillationFrequency;
             
             while(_sinePos >= SINE_TABLE_SIZE)
               {
                  _sinePos -= SINE_TABLE_SIZE;
                  _negSine = !_negSine;
               }
          }
     } // namespace mod
} // namespace mods
