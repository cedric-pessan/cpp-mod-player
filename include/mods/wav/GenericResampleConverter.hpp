#ifndef MODS_WAV_GENERICRESAMPLECONVERTER_HPP
#define MODS_WAV_GENERICRESAMPLECONVERTER_HPP

#include "mods/utils/ConstFraction.hpp"
#include "mods/utils/FirFilterDesigner.hpp"
#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
	class GenericResampleConverter : public WavConverter
          {
           public:
             explicit GenericResampleConverter(WavConverter::ptr src, int inFrequency, int outFrequency);
             
             GenericResampleConverter() = delete;
             GenericResampleConverter(const GenericResampleConverter&) = delete;
             GenericResampleConverter(GenericResampleConverter&&) = delete;
             GenericResampleConverter& operator=(const GenericResampleConverter&) = delete;
             GenericResampleConverter& operator=(GenericResampleConverter&&) = delete;
             ~GenericResampleConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
	     class ResampleSpec
	       {
		public:
		  ResampleSpec(int inputFrequency, int outputFrequency);
		  
		  ResampleSpec() = delete;
		  ResampleSpec(const ResampleSpec&) = default;
		  ResampleSpec(ResampleSpec&&) = delete;
		  ResampleSpec& operator=(const ResampleSpec&) = delete;
		  ResampleSpec& operator=(ResampleSpec&&) = delete;
		  ~ResampleSpec() = default;
		  
		  int getInputFrequency() const;
		  int getOutputFrequency() const;
		  
		private:
		  int _inputFrequency;
		  int _outputFrequency;
	       };
	     
	     static std::unique_ptr<GenericResampleConverter> buildResampleStage(std::vector<ResampleSpec> specsStack, WavConverter::ptr src);
	     
	   protected:
	     GenericResampleConverter(std::vector<ResampleSpec> specsStack, WavConverter::ptr src);
	     
	   private:
	     void init(std::vector<ResampleSpec> specsStack, WavConverter::ptr src);
	     std::vector<ResampleSpec> buildSpecs(int inFrequency, int outFrequency);
	     
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
             
	     mods::utils::ConstFraction _resampleFraction = mods::utils::ConstFraction(1,1);
	     
             const mods::utils::ConstFraction& getResampleFraction() const;
             int getDecimationFactor() const;
             int getInterpolationFactor() const;
	     mods::utils::FirFilterDesigner::ptr _designer;
             
             std::vector<u8> _inputVec;
             mods::utils::RWBuffer<u8> _inputBuffer;
             mods::utils::RWBuffer<double> _inputBufferAsDouble;
             size_t _currentSample;
             
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
		  using ptr = std::unique_ptr<History>;
		  
		  History(int numTaps);
		  
                  History() = delete;
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
		  std::vector<SampleWithZeros> _v;
                  size_t _begin = 0;
                  size_t _end = 0;
               };
	     
	     History::ptr _history;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_GENERICRESAMPLECONVERTER_HPP
