#ifndef MODS_MOD_EFFECTTYPE_HPP
#define MODS_MOD_EFFECTTYPE_HPP

#include "mods/utils/types.hpp"

namespace mods
{
   namespace mod
     {
        enum struct EffectType : u8
          {
             ARPEGGIO = 0x0,
               SLIDE_UP = 0x1,
               SLIDE_DOWN = 0x2,
               SLIDE_TO_NOTE = 0x3,
               VIBRATO = 0x4,
               VIBRATO_AND_VOLUME_SLIDE = 0x6,
               SET_SAMPLE_OFFSET = 0x9,
               VOLUME_SLIDE = 0xa,
               SET_VOLUME = 0xc,
               PATTERN_BREAK = 0xd,
               EXTENDED_EFFECT = 0xe,
               SET_SPEED = 0xf
          };
        
        enum struct ExtendedEffectType : u8
          {
             FINE_SLIDE_UP = 0x1
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_EFFECTTYPE_HPP
