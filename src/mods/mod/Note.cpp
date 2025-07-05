
#include "mods/mod/EffectType.hpp"
#include "mods/mod/Note.hpp"
#include "mods/utils/types.hpp"

#include <cstddef>

namespace mods
{
   namespace mod
     {
        auto Note::getInstrument() const -> size_t
          {
             const u8 nibbleShift = 4;
             const u8 msbNibbleMask = 0xF0U;
             
             const u8 instrumentMSB = _value[0] & msbNibbleMask;
             const u8 instrumentLSB = _value[2] >> nibbleShift;
             
             return instrumentMSB | instrumentLSB;
          }
        
        auto Note::getPeriod() const -> u16
          {
             const u16 periodMSB = (static_cast<u16>(_value[0]) & 0xFU) << 8U;
             const u16 periodLSB = static_cast<u16>(_value[1]);
             
             return periodMSB | periodLSB;
          }
        
        auto Note::getEffect() const -> EffectType
          {
             const u8 nibbleMask = 0xF;
             
             return static_cast<EffectType>(_value[2] & nibbleMask);
          }
        
        auto Note::getEffectArgument() const -> u32
          {
             return _value[3];
          }
        
        auto Note::getExtendedEffect() const -> ExtendedEffectType
          {
             return static_cast<ExtendedEffectType>(_value[3] >> 4);
          }
        
        auto Note::getExtendedEffectArgument() const -> u32
          {
             return _value[3] & 0xF;
          }
     } // namespace mod
} // namespace mods
