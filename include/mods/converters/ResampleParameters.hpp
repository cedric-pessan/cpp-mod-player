#ifndef MODS_CONVERTERS_RESAMPLEPARAMETERS_HPP
#define MODS_CONVERTERS_RESAMPLEPARAMETERS_HPP

#include "mods/StandardFrequency.hpp"
#include "mods/utils/ConstFraction.hpp"
#include "mods/utils/Filters.hpp"
#include "mods/utils/FirFilterDesigner.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace converters
     {
        template<StandardFrequency InFrequency, StandardFrequency OutFrequency>
          class StaticResampleParameters
          {
           public:
             StaticResampleParameters() = default;
             StaticResampleParameters(const StaticResampleParameters&) = default;
             StaticResampleParameters(StaticResampleParameters&&) noexcept = default;
             auto operator=(const StaticResampleParameters&) -> StaticResampleParameters& = delete;
             auto operator=(StaticResampleParameters&&) -> StaticResampleParameters& = delete;
             ~StaticResampleParameters() = default;
             
           private:
             constexpr static int _inFrequency = toUnderlying(InFrequency);
             constexpr static int _outFrequency = toUnderlying(OutFrequency);
             
             constexpr static auto getResampleFraction() -> mods::utils::ConstFraction
               {
                  return mods::utils::ConstFraction(_inFrequency, _outFrequency).reduce();
               }
             
           public:
             constexpr static auto getInterpolationFactor() -> int
               {
                  return getResampleFraction().getDenominator();
               }
             
             constexpr static auto getDecimationFactor() -> int
               {
                  return getResampleFraction().getNumerator();
               }
             
             constexpr static auto isFiltered() -> bool
               {
                  return false;
               }
             
           private:
             using FilterType = mods::utils::LowPassFilter<std::min(_inFrequency, _outFrequency), 2, _inFrequency * getInterpolationFactor()>;
             
           public:
             constexpr static auto getNumTaps() -> int
               {
                  return FilterType::numberOfTaps;
               }
             
             static auto getTap(size_t i) -> double;
             
             constexpr static auto getTaps() -> const typename FilterType::TapsType&
               {
                  return FilterType::getTaps();
               }
          };
        
        class DynamicResampleParameters
          {
           public:
             DynamicResampleParameters(int inFrequency, int outFrequency);
             
             DynamicResampleParameters() = delete;
             DynamicResampleParameters(const DynamicResampleParameters&) = default;
             DynamicResampleParameters(DynamicResampleParameters&&) = default;
             auto operator=(const DynamicResampleParameters&) -> DynamicResampleParameters& = delete;
             auto operator=(DynamicResampleParameters&&) -> DynamicResampleParameters& = delete;
             ~DynamicResampleParameters() = default;
             
             auto getNumTaps() const -> int;
             
             auto getInterpolationFactor() const -> int;
             
             auto getDecimationFactor() const -> int;
             
             auto getTap(size_t i) const -> double;
             
             auto getTaps() const -> const typename mods::utils::FirFilterDesigner::TapsType&;
             
             constexpr static auto isFiltered() -> bool
               {
                  return false;
               }
             
           private:
             auto getResampleFraction() const -> const mods::utils::ConstFraction&;
             
             static constexpr double _nyquistFactor = 2.0;
             static constexpr double _expectedAttenuation = 40.0;
             static constexpr double _transitionWidth = 50.0;
             
             mods::utils::ConstFraction _resampleFraction;
             mods::utils::FirFilterDesigner _designer;
          };
        
        class AmigaResampleParameters
          {
           public:
             AmigaResampleParameters() = default;
             AmigaResampleParameters(const AmigaResampleParameters&) = default;
             AmigaResampleParameters(AmigaResampleParameters&&) = default;
             auto operator=(const AmigaResampleParameters&) -> AmigaResampleParameters& = delete;
             auto operator=(AmigaResampleParameters&&) -> AmigaResampleParameters& = delete;
             ~AmigaResampleParameters() = default;
             
           private:
             constexpr static long _inFrequency = toUnderlying(StandardFrequency::AMIGA);
             constexpr static long _outFrequency = toUnderlying(StandardFrequency::_44100);
             constexpr static long _ledFrequency = toUnderlying(StandardFrequency::AMIGA_LED_CUTOFF);
             
             constexpr static mods::utils::ConstFraction _resampleFraction = mods::utils::ConstFraction(_inFrequency, _outFrequency).reduce();
             constexpr static long _interpolationFactor = _resampleFraction.getDenominator();
             constexpr static long _decimationFactor = _resampleFraction.getNumerator();
             
             using DefaultFilterType = mods::utils::LowPassFilter<std::min(_inFrequency, _outFrequency), 2, _inFrequency * _interpolationFactor>;
             using LedFilterType = mods::utils::LowPassFilter<_ledFrequency, 1, _inFrequency * _interpolationFactor>;
             
           public:
             constexpr static auto getInterpolationFactor() noexcept -> long
               {
                  return _interpolationFactor;
               }
             
             constexpr static auto getDecimationFactor() noexcept -> long
               {
                  return _decimationFactor;
               }
             
             constexpr static auto getNumTaps() noexcept -> int
               {
                  static_assert(DefaultFilterType::numberOfTaps == LedFilterType::numberOfTaps, "Both default filter and led filter should have the same number of taps");
                  return DefaultFilterType::numberOfTaps;
               }
             
             static auto getTap(size_t i) -> double;
             static auto getFilteredTap(size_t i) -> double;
             
             static auto getTaps() -> const typename DefaultFilterType::TapsType&;
             static auto getFilteredTaps() -> const typename LedFilterType::TapsType&;
             
             auto getAndConsumeCurrentSampleLength() -> u32;
             auto getCurrentSampleLength() const -> u32;
             auto getCurrentSampleValue() const -> double;
             auto isFiltered() const -> bool;
             void setFiltered(bool filtered);
             void setCurrentSampleValue(double value);
             void setCurrentSampleLength(u32 length);
             
           private:
             u32 _currentSampleLength = 0;
             double _currentSampleValue = 0.0;
             bool _currentSampleFiltered = false;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_RESAMPLEPARAMETERS_HPP
