
#include "mods/mod/Retrigger.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace mod
     {
        void Retrigger::init(u32 retriggerPeriod)
          {
             _tickCount = 0;
             _retriggerPeriod = retriggerPeriod;
          }
        
        auto Retrigger::getModifiedPeriod(u16 period) -> u16
          {
             return period;
          }
        
        auto Retrigger::getModifiedVolume(u16 volume) const -> u16
          {
             return volume;
          }
        
        auto Retrigger::retriggerSample() const -> bool
          {
             return _tickCount == _retriggerPeriod - 1;
          }
        
        auto Retrigger::isSampleEnabled() const -> bool
          {
             return true;
          }
        
        void Retrigger::tick()
          {
             ++_tickCount;
             if(_tickCount > _retriggerPeriod)
               {
                  _tickCount = 0;
               }
          }
     } // namespace mod
} // namespace mods
