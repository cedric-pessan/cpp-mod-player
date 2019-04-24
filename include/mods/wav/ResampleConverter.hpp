#ifndef MODS_WAV_RESAMPLECONVERTER_HPP
#define MODS_WAV_RESAMPLECONVERTER_HPP

#include "mods/utils/ConstFraction.hpp"
#include "mods/utils/Filters.hpp"
#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<int InFrequency, int OutFrequency>
          class ResampleConverter : public WavConverter
          {
           public:
             explicit ResampleConverter(WavConverter::ptr src);
             
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
             double calculateInterpolatedSample() const;
             double getNextSample();
             bool nextSampleExists();
             mods::utils::RWBuffer<u8> initBuffer();
             
             WavConverter::ptr _src;
             int _zerosToNextInterpolatedSample = 0;
             
             constexpr static mods::utils::ConstFraction _resampleFraction = mods::utils::ConstFraction(InFrequency, OutFrequency).reduce();
             constexpr static int _decimationFactor = _resampleFraction.getNumerator();
             constexpr static int _interpolationFactor = _resampleFraction.getDenominator();
             using FilterType = mods::utils::LowPassFilter<std::min(InFrequency, OutFrequency), 2, InFrequency * _interpolationFactor>;
             constexpr static int _numTaps = FilterType::taps.size();
             
             std::array<u8, _numTaps * sizeof(double)> _inputArray;
             mods::utils::RWBuffer<u8> _inputBuffer;
             mods::utils::RWBuffer<double> _inputBufferAsDouble;
             size_t _currentSample = _numTaps;
             
             struct SampleWithZeros
               {
                  SampleWithZeros(double sample, int zeros);
                  
                  SampleWithZeros() = default;
                  SampleWithZeros(const SampleWithZeros&) = delete;
                  SampleWithZeros(SampleWithZeros&&) = delete;
                  SampleWithZeros& operator=(const SampleWithZeros&) = default;
                  SampleWithZeros& operator=(SampleWithZeros&&) = delete;
                  ~SampleWithZeros() = default;
                  
                  int numberOfZeros;
                  double sample;
               };
             
             class History
               {
                public:
                  History() = default;
                  History(const History&) = delete;
                  History(History&&) = delete;
                  History& operator=(const History&) = delete;
                  History& operator=(History&&) = delete;
                  ~History() = default;
                  
                  void push_back(const SampleWithZeros& sampleWithZeros);
                  SampleWithZeros& front();
                  void pop_front();
                  bool isEmpty() const;
                  const SampleWithZeros& getSample(size_t i) const;
                  
                private:
                  std::array<SampleWithZeros, _numTaps> _array;
                  size_t _begin = 0;
                  size_t _end = 0;
               } _history;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_RESAMPLECONVERTER_HPP
