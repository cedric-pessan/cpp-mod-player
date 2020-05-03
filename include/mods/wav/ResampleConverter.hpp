#ifndef MODS_WAV_RESAMPLECONVERTER_HPP
#define MODS_WAV_RESAMPLECONVERTER_HPP

#include "mods/StandardFrequency.hpp"
#include "mods/utils/ConstFraction.hpp"
#include "mods/utils/Filters.hpp"
#include "mods/utils/FirFilterDesigner.hpp"
#include "mods/wav/WavConverter.hpp"
#include "mods/wav/impl/ResampleConverterImpl.hpp"

namespace mods
{
   namespace wav
     {
        template<typename PARAMETERS>
          class ResampleConverter : public WavConverter
          {
           public:
             explicit ResampleConverter(WavConverter::ptr src, PARAMETERS parameters);
             
             ResampleConverter() = delete;
             ResampleConverter(const ResampleConverter&) = delete;
             ResampleConverter(ResampleConverter&&) = delete;
             auto operator=(const ResampleConverter&) -> ResampleConverter& = delete;
             auto operator=(ResampleConverter&&) -> ResampleConverter& = delete;
             ~ResampleConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             auto getNextDecimatedSample() -> double;
             void updateHistory();
             void removeFromHistory();
             void addToHistory();
             auto calculateInterpolatedSample() -> double;
             auto getNextSample() -> double;
             auto nextSampleExists() const -> bool;
             auto initBuffer() -> mods::utils::RWBuffer<u8>;
             
             WavConverter::ptr _src;
             PARAMETERS _resampleParameters;
             int _zerosToNextInterpolatedSample = 0;
             
             std::vector<u8> _inputVec;
             mods::utils::RWBuffer<u8> _inputBuffer;
             mods::utils::RWBuffer<double> _inputBufferAsDouble;
             size_t _currentSample;
             
             impl::History _history;
          };
        
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

#endif // MODS_WAV_RESAMPLECONVERTER_HPP
