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
             
           private:
             auto getResampleFraction() const -> const mods::utils::ConstFraction&;
             
             static constexpr double _nyquistFactor = 2.0;
             static constexpr double _expectedAttenuation = 40.0;
             static constexpr double _transitionWidth = 50.0;
             
             mods::utils::ConstFraction _resampleFraction;
             mods::utils::FirFilterDesigner _designer;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_RESAMPLEPARAMETERS_HPP
