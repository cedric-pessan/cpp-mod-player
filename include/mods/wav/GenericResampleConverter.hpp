#ifndef MODS_WAV_GENERICRESAMPLECONVERTER_HPP
#define MODS_WAV_GENERICRESAMPLECONVERTER_HPP

#include "mods/utils/ConstFraction.hpp"
#include "mods/utils/FirFilterDesigner.hpp"
#include "mods/wav/WavConverter.hpp"

#include <cmath>
#include <fstream>

namespace mods
{
   namespace wav
     {
        namespace
          {
             std::array<double,30> i0_0_8_coefficients
               {
                  -4.41534164647933937950E-18,
                    3.33079451882223809783E-17,
                    -2.43127984654795469359E-16,
                    1.71539128555513303061E-15,
                    -1.16853328779934516808E-14,
                    7.67618549860493561688E-14,
                    -4.85644678311192946090E-13,
                    2.95505266312963983461E-12,
                    -1.72682629144155570723E-11,
                    9.67580903537323691224E-11,
                    -5.18979560163526290666E-10,
                    2.65982372468238665035E-9,
                    -1.30002500998624804212E-8,
                    6.04699502254191894932E-8,
                    -2.67079385394061173391E-7,
                    1.11738753912010371815E-6,
                    -4.41673835845875056359E-6,
                    1.64484480707288970893E-5,
                    -5.75419501008210370398E-5,
                    1.88502885095841655729E-4,
                    -5.76375574538582365885E-4,
                    1.63947561694133579842E-3,
                    -4.32430999505057594430E-3,
                    1.05464603945949983183E-2,
                    -2.37374148058994688156E-2,
                    4.93052842396707084878E-2,
                    -9.49010970480476444210E-2,
                    1.71620901522208775349E-1,
                    -3.04682672343198398683E-1,
                    6.76795274409476084995E-1
               };
             
             std::array<double,25> i0_8_infinity_coefficients
               {
                  -7.23318048787475395456E-18,
                    -4.83050448594418207126E-18,
                    4.46562142029675999901E-17,
                    3.46122286769746109310E-17,
                    -2.82762398051658348494E-16,
                    -3.42548561967721913462E-16,
                    1.77256013305652638360E-15,
                    3.81168066935262242075E-15,
                    -9.55484669882830764870E-15,
                    -4.15056934728722208663E-14,
                    1.54008621752140982691E-14,
                    3.85277838274214270114E-13,
                    7.18012445138366623367E-13,
                    -1.79417853150680611778E-12,
                    -1.32158118404477131188E-11,
                    -3.14991652796324136454E-11,
                    1.18891471078464383424E-11,
                    4.94060238822496958910E-10,
                    3.39623202570838634515E-9,
                    2.26666899049817806459E-8,
                    2.04891858946906374183E-7,
                    2.89137052083475648297E-6,
                    6.88975834691682398426E-5,
                    3.36911647825569408990E-3,
                    8.04490411014108831608E-1
               };
             
             template<size_t N>
               double evalChebyshev(double x, const std::array<double, N>& coefficients)
                 {
                    auto it = coefficients.begin();
                    auto b0 = *it;
                    ++it;
                    
                    double b1 = 0.0;
                    double b2 = 0.0;
                    
                    for(;it != coefficients.end(); ++it)
                      {
                         b2 = b1;
                         b1 = b0;
                         b0 = x * b1 - b2 + *it;
                      }
                    
                    return 0.5 * (b0 - b2);
                 }
             
             double i0(double x)
               {
                  if(x < 0)
                    {
                       return i0(-x);
                    }
                  if(x <= 8.0)
                    {
                       double y = (x / 2.0) - 2.0;
                       return std::exp(x) * evalChebyshev(y, i0_0_8_coefficients);
                    }
                  
                  return std::exp(x) * evalChebyshev(32.0 / x - 2.0, i0_8_infinity_coefficients) / std::sqrt(x);
               }
          }
        
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
                       double kaiserValue = i0(beta * std::sqrt(1 - std::pow((i - alpha) / alpha, 2.0))) / i0(beta);
                       
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
