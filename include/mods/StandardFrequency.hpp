#ifndef MODS_STANDARDFREQUENCY_HPP
#define MODS_STANDARDFREQUENCY_HPP

namespace mods
{
   enum struct StandardFrequency : int
     {
        _8000 = 8000,
          _10000 = 10000,
          _11025 = 11025,
          _22000 = 22000,
          _22050 = 22050,
          _44100 = 44100,
          _48000 = 48000,
          
          AMIGA = 3546895,
          AMIGA_LED_CUTOFF = 3275
     };
} // namespace mods

#endif // MODS_STANDARDFREQUENCY_HPP
