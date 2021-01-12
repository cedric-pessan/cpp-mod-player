#ifndef MODS_WAV_RESAMPLEPARAMETERS_HPP
#define MODS_WAV_RESAMPLEPARAMETERS_HPP

#include "mods/StandardFrequency.hpp"
#include "mods/utils/ConstFraction.hpp"
#include "mods/utils/Filters.hpp"
#include "mods/utils/FirFilterDesigner.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace wav
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
                  return FilterType::taps.size();
               }
             
             constexpr static auto getTap(size_t i) -> double
               {
                  using mods::utils::at;
                  return at(FilterType::taps, i);
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
             
           private:
             auto getResampleFraction() const -> const mods::utils::ConstFraction&;
             
             static constexpr double _nyquistFactor = 2.0;
             
             mods::utils::ConstFraction _resampleFraction;
             mods::utils::FirFilterDesigner _designer;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_RESAMPLEPARAMETERS_HPP
