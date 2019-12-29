#ifndef MODS_WAV_GENERICRESAMPLECONVERTER_HPP
#define MODS_WAV_GENERICRESAMPLECONVERTER_HPP

/*#include "mods/utils/ConstFraction.hpp"*/
#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
	class GenericResampleConverter : public WavConverter
          {
           public:
             explicit GenericResampleConverter(WavConverter::ptr src);
             
             GenericResampleConverter() = delete;
             GenericResampleConverter(const GenericResampleConverter&) = delete;
             GenericResampleConverter(GenericResampleConverter&&) = delete;
             GenericResampleConverter& operator=(const GenericResampleConverter&) = delete;
             GenericResampleConverter& operator=(GenericResampleConverter&&) = delete;
             ~GenericResampleConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             /*double getNextDecimatedSample();
             void updateHistory();
             void removeFromHistory();
             void addToHistory();
             double calculateInterpolatedSample() const;
             double getNextSample();
             bool nextSampleExists();
             mods::utils::RWBuffer<u8> initBuffer();*/
             
             WavConverter::ptr _src;
             /*int _zerosToNextInterpolatedSample = 0;
             
             constexpr static mods::utils::ConstFraction getResampleFraction()
               {
                  return mods::utils::ConstFraction(InFrequency, OutFrequency).reduce();
               }
             constexpr static int getDecimationFactor()
               {
                  return getResampleFraction().getNumerator();
               }
             constexpr static int getInterpolationFactor()
               {
                  return getResampleFraction().getDenominator();
               }
             using FilterType = mods::utils::LowPassFilter<std::min(InFrequency, OutFrequency), 2, InFrequency * getInterpolationFactor()>;
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
                  std::array<SampleWithZeros, _numTaps> _array = {};
                  size_t _begin = 0;
                  size_t _end = 0;
               } _history;*/
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_GENERICRESAMPLECONVERTER_HPP
