
#include "mods/mod/DelaySample.hpp"

namespace mods
{
   namespace mod
     {
        void DelaySample::init(int delayCounter)
          {
             _retrigger = false;
             _sampleEnabled = false;
             _delayCounter = delayCounter;
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