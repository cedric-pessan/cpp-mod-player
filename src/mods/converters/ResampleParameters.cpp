
#include "mods/converters/ResampleParameters.hpp"

namespace mods
{
   namespace converters
     {
        template<StandardFrequency InFrequency, StandardFrequency OutFrequency>
          auto StaticResampleParameters<InFrequency, OutFrequency>::getTap(size_t i) -> double
          {
             return FilterType::getTap(i);
          }
        
        DynamicResampleParameters::DynamicResampleParameters(int inFrequency, int outFrequency)
          : _resampleFraction(mods::utils::ConstFraction(inFrequency, outFrequency).reduce()),
          _designer(inFrequency * getInterpolationFactor(), // sampleFrequency
                    std::min(inFrequency, outFrequency) / _nyquistFactor, _expectedAttenuation, _transitionWidth) // cutoffFrequency
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
        
        template class StaticResampleParameters<StandardFrequency::_22000, StandardFrequency::_44100>;
        template class StaticResampleParameters<StandardFrequency::_8000,  StandardFrequency::_44100>;
        template class StaticResampleParameters<StandardFrequency::_48000, StandardFrequency::_44100>;
        template class StaticResampleParameters<StandardFrequency::_10000, StandardFrequency::_44100>;
     } // namespace converters
} // namespace mods
