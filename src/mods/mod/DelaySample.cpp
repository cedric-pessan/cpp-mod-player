
#include "mods/mod/DelaySample.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace mod
     {
        void DelaySample::init(u32 delayCounter)
          {
             _retrigger = false;
             _sampleEnabled = false;
             _delayCounter = static_cast<int>(delayCounter);
          }
        
        auto DelaySample::getModifiedPeriod(u16 period) -> u16
          {
             return period;
          }
        
        auto DelaySample::getModifiedVolume(u16 volume) const -> u16
          {
             return volume;
          }
        
        auto DelaySample::retriggerSample() const -> bool
          {
             return _retrigger;
          }
        
        auto DelaySample::isSampleEnabled() const -> bool
          {
             return _sampleEnabled;
          }
        
        void DelaySample::tick()
          {
             if(!_sampleEnabled)
               {
                  --_delayCounter;
                  if(_delayCounter <= 0)
                    {
                       _sampleEnabled = true;
                       _retrigger = true;
                    }
               }
             else
               {
                  if(_retrigger)
                    {
                       _retrigger = false;
                    }
               }
          }
     } // namespace mod
} // namespace mods