
#include "mods/wav/ResampleParameters.hpp"

namespace mods
{
   namespace wav
     {
        DynamicResampleParameters::DynamicResampleParameters(int inFrequency, int outFrequency)
          : _resampleFraction(mods::utils::ConstFraction(inFrequency, outFrequency).reduce()),
          _designer(inFrequency * getInterpolationFactor(), // sampleFrequency
                    std::min(inFrequency, outFrequency) / _nyquistFactor) // cutoffFrequency
            {
            }
        
        auto DynamicResampleParameters::getNumTaps() const -> int
          {
             return _designer.getTaps().size();
          }
        
        auto DynamicResampleParameters::getResampleFraction() const -> const mods::utils::ConstFraction&
          {
             return _resampleFraction;
          }
        
        auto DynamicResampleParameters::getInterpolationFactor() const -> int
          {
             return getResampleFraction().getDenominator();
          }
        
        auto DynamicResampleParameters::getDecimationFactor() const -> int
          {
             return getResampleFraction().getNumerator();
          }
        
        auto DynamicResampleParameters::getTap(size_t i) const -> double
          {
             return _designer.getTaps()[i];
          }
        
        auto DynamicResampleParameters::getTaps() const -> const typename mods::utils::FirFilterDesigner::TapsType&
          {
             return _designer.getTaps();
          }
     } // namespace wav
} // namespace mods
