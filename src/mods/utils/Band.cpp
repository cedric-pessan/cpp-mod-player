
#include "mods/utils/Band.hpp"

#include <cmath>

namespace mods
{
   namespace utils
     {
        Band::Band(double startHz, double stopHz, double gain, double ripple, double sampleFrequency)
          : _gain(gain)
          {
             _start = startHz / sampleFrequency * 2.0;
             _stop = stopHz / sampleFrequency * 2.0;
             
             if(gain != 0.0)
               {
                  _weight = std::pow(10.0, -ripple / M_E / 20.0);
                  
                  _upper = 1.0 / _weight * gain;
                  _lower = (1.0-(1.0/_weight-1.0)) * gain;
                  _weight = 1.0 / (1.0 / _weight - 1.0) / gain;
               }
             else
               {
                  _weight = std::pow(10.0, -ripple / 20.0);
                  _upper = std::pow(10.0l, ripple / 20.0);
               }
          }
        
        double Band::getStart() const
          {
             return _start;
          }
        
        double Band::getStop() const
          {
             return _stop;
          }
        
        double Band::getGain() const
          {
             return _gain;
          }
        
        double Band::getWeight() const
          {
             return _weight;
          }
        
        double Band::getUpper() const
          {
             return _upper;
          }
        
        double Band::getLower() const
          {
             return _lower;
          }
     } // namespace utils
} // namespace mods