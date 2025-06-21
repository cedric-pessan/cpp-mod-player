
#include "mods/StandardFrequency.hpp"
#include "mods/converters/ResampleParameters.hpp"
#include "mods/utils/ConstFraction.hpp"
#include "mods/utils/FirFilterDesigner.hpp"
#include "mods/utils/types.hpp"

#include <algorithm>
#include <cstddef>

namespace mods
{
   namespace converters
     {
        template<StandardFrequency InFrequency, StandardFrequency OutFrequency>
          auto StaticResampleParameters<InFrequency, OutFrequency>::getTap(size_t tapIndex) -> double
          {
             return FilterType::getTap(tapIndex);
          }
        
        DynamicResampleParameters::DynamicResampleParameters(u32 inFrequency, u32 outFrequency)
          : _resampleFraction(mods::utils::ConstFraction(static_cast<mods::utils::ConstFraction::Numerator>(inFrequency),
                                                         static_cast<mods::utils::ConstFraction::Denominator>(outFrequency)).reduce()),
          _designer(mods::utils::FirFilterDesigner::Params{static_cast<u64>(inFrequency) * getInterpolationFactor(), // sampleFrequency
               std::min(inFrequency, outFrequency) / _nyquistFactor, // cutoffFrequency
               _expectedAttenuation, _transitionWidth})
            {
            }
        
        auto DynamicResampleParameters::getNumTaps() const -> size_t
          {
             return _designer.getTaps().size();
          }
        
        auto DynamicResampleParameters::getResampleFraction() const -> const mods::utils::ConstFraction&
          {
             return _resampleFraction;
          }
        
        auto DynamicResampleParameters::getInterpolationFactor() const -> u64
          {
             return getResampleFraction().getDenominator();
          }
        
        auto DynamicResampleParameters::getDecimationFactor() const -> u64
          {
             return getResampleFraction().getNumerator();
          }
        
        auto DynamicResampleParameters::getTap(size_t tapIndex) const -> double
          {
             return _designer.getTaps()[tapIndex];
          }
        
        auto DynamicResampleParameters::getTaps() const -> const typename mods::utils::FirFilterDesigner::TapsType&
          {
             return _designer.getTaps();
          }
        
        auto AmigaResampleParameters::getCurrentSampleLength() const -> u32
          {
             return _currentSampleLength;
          }
        
        auto AmigaResampleParameters::getAndConsumeCurrentSampleLength() -> u32
          {
             if(_currentSampleLength == 0)
               {
                  return 0;
               }
             return _currentSampleLength--;
          }
        
        auto AmigaResampleParameters::getCurrentSampleValue() const -> double
          {
             return _currentSampleValue;
          }
        
        auto AmigaResampleParameters::isFiltered() const -> bool
          {
             return _currentSampleFiltered;
          }
        
        void AmigaResampleParameters::setFiltered(bool filtered)
          {
             _currentSampleFiltered = filtered;
          }
        
        void AmigaResampleParameters::setCurrentSampleValue(double value)
          {
             _currentSampleValue = value;
          }
        
        void AmigaResampleParameters::setCurrentSampleLength(u32 length)
          {
             _currentSampleLength = length;
          }
        
        auto AmigaResampleParameters::getTaps() -> const typename DefaultFilterType::TapsType&
          {
             return DefaultFilterType::getTaps();
          }
        
        auto AmigaResampleParameters::getFilteredTaps() -> const typename LedFilterType::TapsType&
          {
             return LedFilterType::getTaps();
          }
        
        auto AmigaResampleParameters::getTap(size_t tapIndex) -> double
          {
             return DefaultFilterType::getTap(tapIndex);
          }
        
        auto AmigaResampleParameters::getFilteredTap(size_t tapIndex) -> double
          {
             return LedFilterType::getTap(tapIndex);
          }
        
        template class StaticResampleParameters<StandardFrequency::_22000, StandardFrequency::_44100>;
        template class StaticResampleParameters<StandardFrequency::_8000,  StandardFrequency::_44100>;
        template class StaticResampleParameters<StandardFrequency::_48000, StandardFrequency::_44100>;
        template class StaticResampleParameters<StandardFrequency::_10000, StandardFrequency::_44100>;
     } // namespace converters
} // namespace mods
