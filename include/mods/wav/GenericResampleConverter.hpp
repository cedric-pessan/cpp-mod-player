#ifndef MODS_WAV_GENERICRESAMPLECONVERTER_HPP
#define MODS_WAV_GENERICRESAMPLECONVERTER_HPP

#include "mods/utils/bessel.hpp"
#include "mods/utils/ConstFraction.hpp"
#include "mods/utils/FirFilterDesigner.hpp"
#include "mods/wav/WavConverter.hpp"

#include <cmath>

namespace mods
{
   namespace wav
     {
         class FakeDesigner
          {
           public:
             FakeDesigner(int sampleFrequency, int cutOff)
               : _sampleFrequency(sampleFrequency),
               _cutOff(cutOff)
               {
               }
             
             void displayProgress()
               {
               }
             
             void optimizeFilter(int numTaps)
               {
                  /*std::ifstream f("/home/cedric/projects/testfilter/kaiser/coefficients.txt");
                  std::string line;
                  while(std::getline(f, line))
                    {
                       double d = std::stod(line, nullptr);
                       _taps.push_back(d);
                    }
                  return;*/
                  
                  double A = 40.0;
                  double transitionWidth = 50.0;
                  double niquist = _sampleFrequency / 2.0;
                  double deltaOmega = transitionWidth / niquist * M_PI;
                  //double M = ((A - 7.95) / (2.285 * deltaOmega)) + 1;
                  int M = static_cast<int>(((A - 7.95) / (2.285 * deltaOmega)) + 0.5);
                  if((M&1) == 0) ++M;
                  std::cout << "M=" << M << std::endl;
                  
                  /*for(int i=0; i<M; ++i)
                    _taps.push_back(1.0 / static_cast<double>(M));*/
                  
                  double wc = _cutOff / (_sampleFrequency /*/ 2.0*/);
                  
                  for(int i=0; i<M; ++i)
                    {
                       if((i-(M/2)) == 0)
                         {
                            _taps.push_back(wc * 2.0);
                         }
                       else
                         {
                            _taps.push_back(std::sin(2.0*M_PI*wc*(i-M/2)) / (M_PI*(i-(M/2))));
                         }
                    }
                  
                  // blackman window
                  /*for(int i=0; i<M; ++i)
                    {
                       _taps[i] *= 0.5 + 0.5 * std::cos(2.0 * M_PI * (double)(i-M/2) / M);
                    }*/
                  
                  // kaiser window
                  
                  // compute beta parameter
                  double beta = 0.0;
                  if(A > 50)
                    {
                       beta = 0.1102 * (A - 8.7);
                    }
                  else if(A > 21)
                    {
                       beta = 0.5842 * std::pow(A - 21, 0.4) + 0.07886 * (A - 21);
                    }
                  
                  double alpha = M / 2;
                  for(int i=0; i<M; ++i)
                    {
                       namespace bessel = mods::utils::bessel;
                       double kaiserValue = bessel::i0(beta * std::sqrt(1 - std::pow((i - alpha) / alpha, 2.0))) / bessel::i0(beta);
                       
                       _taps[i] *= kaiserValue;
                    }
               }
             
             const std::vector<double>& getTaps() const
               {
                  return _taps;
               }
             
           private:
             std::vector<double> _taps;
             
             double _sampleFrequency;
             double _cutOff;
          };
        
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
	     //mods::utils::FirFilterDesigner::ptr _designer;
             std::unique_ptr<FakeDesigner> _designer;
             
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
                  SampleWithZeros& back();
                  void pop_front();
                  bool isEmpty() const;
                  const SampleWithZeros& getSample(size_t i);
                  
                private:
		  std::vector<SampleWithZeros> _v;
                  size_t _begin = 0;
                  size_t _end = 0;
                  
                  SampleWithZeros _zeros;
               };
	     
	     History::ptr _history;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_GENERICRESAMPLECONVERTER_HPP
