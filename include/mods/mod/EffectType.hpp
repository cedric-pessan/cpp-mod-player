#ifndef MODS_MOD_EFFECTTYPE_HPP
#define MODS_MOD_EFFECTTYPE_HPP

namespace mods
{
   namespace mod
     {
        enum struct EffectType : u8
          {
             ARPEGGIO = 0x0,
               SLIDE_UP = 0x1,
               SLIDE_DOWN = 0x2,
               VIBRATO = 0x4,
               VIBRATO_AND_VOLUME_SLIDE = 0x6,
               SET_SAMPLE_OFFSET = 0x9,
               VOLUME_SLIDE = 0xa,
               SET_VOLUME = 0xc,
               PATTERN_BREAK = 0xd,
               SET_SPEED = 0xf
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_EFFECTTYPE_HPP
