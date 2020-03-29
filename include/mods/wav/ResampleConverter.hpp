#ifndef MODS_WAV_RESAMPLECONVERTER_HPP
#define MODS_WAV_RESAMPLECONVERTER_HPP

#include "mods/utils/ConstFraction.hpp"
#include "mods/utils/Filters.hpp"
#include "mods/utils/FirFilterDesigner.hpp"
#include "mods/wav/impl/ResampleConverterImpl.hpp"
#include "mods/wav/WavConverter.hpp"

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
             ResampleConverter& operator=(const ResampleConverter&) = delete;
             ResampleConverter& operator=(ResampleConverter&&) = delete;
             ~ResampleConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             double getNextDecimatedSample();
             void updateHistory();
             void removeFromHistory();
             void addToHistory();
             double calculateInterpolatedSample();
             double getNextSample();
             bool nextSampleExists() const;
             mods::utils::RWBuffer<u8> initBuffer();
             
             WavConverter::ptr _src;
             PARAMETERS _resampleParameters;
             int _zerosToNextInterpolatedSample = 0;
             
             std::vector<u8> _inputVec;
             mods::utils::RWBuffer<u8> _inputBuffer;
             mods::utils::RWBuffer<double> _inputBufferAsDouble;
             size_t _currentSample;
             
             impl::History _history;
          };
        
        template<int InFrequency, int OutFrequency>
          class StaticResampleParameters 
          {
           public:
             StaticResampleParameters() = default;
             StaticResampleParameters(const StaticResampleParameters&) = default;
             StaticResampleParameters(StaticResampleParameters&&) = default;
             StaticResampleParameters& operator=(const StaticResampleParameters&) = delete;
             StaticResampleParameters& operator=(StaticResampleParameters&&) = delete;
             ~StaticResampleParameters() = default;
             
           private:
             constexpr static mods::utils::ConstFraction getResampleFraction()
               {
                  return mods::utils::ConstFraction(InFrequency, OutFrequency).reduce();
               }
             
           public:
             constexpr static int getInterpolationFactor()
               {
                  return getResampleFraction().getDenominator();
               }
             
             constexpr static int getDecimationFactor()
               {
                  return getResampleFraction().getNumerator();
               }
             
           private:
             using FilterType = mods::utils::LowPassFilter<std::min(InFrequency, OutFrequency), 2, InFrequency * getInterpolationFactor()>;
             
           public:
             constexpr static int getNumTaps()
               {
                  return FilterType::taps.size();
               }
             
             constexpr static double getTap(size_t i)
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
             DynamicResampleParameters& operator=(const DynamicResampleParameters&) = delete;
             DynamicResampleParameters& operator=(DynamicResampleParameters&&) = delete;
             ~DynamicResampleParameters() = default;
             
             int getNumTaps() const;
             
             int getInterpolationFactor() const;
             
             int getDecimationFactor() const;
             
             double getTap(size_t i) const;
             
           private:
             const mods::utils::ConstFraction& getResampleFraction() const;
             
             mods::utils::ConstFraction _resampleFraction = mods::utils::ConstFraction(1,1);
             mods::utils::FirFilterDesigner _designer;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_RESAMPLECONVERTER_HPP
