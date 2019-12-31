
#include "mods/utils/integerFactorization.hpp"
#include "mods/wav/GenericResampleConverter.hpp"

#include <algorithm>

namespace mods
{
   namespace wav
     {
	GenericResampleConverter::GenericResampleConverter(WavConverter::ptr src, int inFrequency, int outFrequency)
          /*: _inputBuffer(initBuffer()),
          _inputBufferAsDouble(_inputBuffer.slice<double>(0, _numTaps))*/
	  {
	     auto specsStack = buildSpecs(inFrequency, outFrequency);
	     init(std::move(specsStack), std::move(src));
	  }
	
	GenericResampleConverter::GenericResampleConverter(std::vector<ResampleSpec> specsStack, WavConverter::ptr src)
	  {
	     init(std::move(specsStack), std::move(src));
	  }
	
	std::unique_ptr<GenericResampleConverter> GenericResampleConverter::buildResampleStage(std::vector<ResampleSpec> specsStack, WavConverter::ptr src)
	  {
	     class make_unique_enabler : public GenericResampleConverter
	       {
		public:
		  make_unique_enabler(std::vector<ResampleSpec> specsStack, WavConverter::ptr src)
		    : GenericResampleConverter(std::move(specsStack), std::move(src))
		    {
		    }
		  
		  make_unique_enabler() = delete;
		  make_unique_enabler(const make_unique_enabler&) = delete;
		  make_unique_enabler(make_unique_enabler&&) = delete;
		  make_unique_enabler& operator=(const make_unique_enabler&) = delete;
		  make_unique_enabler& operator=(make_unique_enabler&&) = delete;
		  ~make_unique_enabler() override = default;
	       };
	     return std::make_unique<make_unique_enabler>(std::move(specsStack), std::move(src));
	  }
	
	void GenericResampleConverter::init(std::vector<ResampleSpec> specsStack, WavConverter::ptr src)
	  {
	     ResampleSpec spec = specsStack.back();
	     specsStack.pop_back();
	     
	     int inFrequency = spec.getInputFrequency();
	     int outFrequency = spec.getOutputFrequency();
	     
	     std::cout << "compute resampler from " << inFrequency << "Hz to " << outFrequency << std::endl;
	     
	     _resampleFraction = mods::utils::ConstFraction(inFrequency, outFrequency).reduce();
	     
	     double cutoffFrequency = std::min(inFrequency, outFrequency) / 2.0;
	     double sampleFrequency = static_cast<double>(inFrequency) * static_cast<double>(getInterpolationFactor());
	     
	     std::vector<mods::utils::Band> bands;
	     bands.emplace_back(0.0, cutoffFrequency, 1.0, 5.0, sampleFrequency);
	     bands.emplace_back(cutoffFrequency + 50, sampleFrequency / 2.0, 0.0, -40.0, sampleFrequency);
	     
	     _designer = std::make_unique<mods::utils::FirFilterDesigner>(bands);
	     _designer->optimizeFilter(1401);
	     
	     auto& taps = _designer->getTaps();
	     _history = std::make_unique<History>(taps.size());
	     _history->push_back(SampleWithZeros(0.0,taps.size()-1));
	     
	     if(specsStack.empty())
	       {
		  _src = std::move(src);
	       }
	     else
	       {
		  _src = buildResampleStage(std::move(specsStack), std::move(src));
	       }
	  }
	
	std::vector<GenericResampleConverter::ResampleSpec> GenericResampleConverter::buildSpecs(int inFrequency, int outFrequency)
	  {
	     auto globalResampleFraction = mods::utils::ConstFraction(inFrequency, outFrequency).reduce();
	     auto globalInterpolationFactor = globalResampleFraction.getDenominator();
	     auto globalDecimationFactor = globalResampleFraction.getNumerator();
	     
	     auto interpolationFactors = mods::utils::integerFactorization(globalInterpolationFactor);
	     auto decimationFactors = mods::utils::integerFactorization(globalDecimationFactor);
	     
	     while(interpolationFactors.size() < decimationFactors.size())
	       {
		  interpolationFactors.push_back(1);
	       }
	     while(decimationFactors.size() < interpolationFactors.size())
	       {
		  decimationFactors.push_back(1);
	       }
	     
	     std::sort(interpolationFactors.begin(), interpolationFactors.end());
	     std::sort(decimationFactors.begin(), decimationFactors.end());
	     
	     auto itInterpolation = interpolationFactors.rbegin();
	     auto itDecimation = decimationFactors.begin();
	     
	     std::vector<ResampleSpec> specs;
	     
	     int currentFrequency = inFrequency;
	     
	     while((itInterpolation != interpolationFactors.rend()) ||
		   (itDecimation != decimationFactors.end()))
	       {
		  int nextFrequency = currentFrequency * *itInterpolation;
		  nextFrequency /= *itDecimation;
		  specs.emplace_back(currentFrequency, nextFrequency);
		  currentFrequency = nextFrequency;
		  
		  if(itInterpolation != interpolationFactors.rend())
		    {
		       ++itInterpolation;
		    }
		  if(itDecimation != decimationFactors.end())
		    {
		       ++itDecimation;
		    }
	       }
	     
	     return specs;
	  }
        
        /*template<int InFrequency, int OutFrequency>
          mods::utils::RWBuffer<u8> ResampleConverter<InFrequency, OutFrequency>::initBuffer()
          {
             u8* ptr = _inputArray.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, _numTaps * sizeof(double), std::move(deleter));
             return mods::utils::RWBuffer<u8>(buffer);
          }*/
        
	bool GenericResampleConverter::isFinished() const
          {
             return _src->isFinished() && _history->isEmpty();
          }
        
	void GenericResampleConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
	  {
	     /*if((len % sizeof(double)) != 0)
	       {
		  std::cout << "TODO: wrong buffer length in ResampleConverter" << std::endl;
	       }
	     
	     int nbElems = len / sizeof(double);
	     
	     auto outView = buf->slice<double>(0, nbElems);
	     
	     for(int i=0; i<nbElems; ++i)
	       {
		  outView[i] = getNextDecimatedSample();
	       }*/
	     _src->read(buf, len);
	     std::cout << "TODO: GenericResampleConverter::read(mods::utils::RWBuffer<u8>*, int) const" << std::endl;
	  }
        
        /*template<int InFrequency, int OutFrequency>
          double ResampleConverter<InFrequency, OutFrequency>::getNextDecimatedSample()
            {
               updateHistory();
               return calculateInterpolatedSample();
            }
        
        template<int InFrequency, int OutFrequency>
          void ResampleConverter<InFrequency, OutFrequency>::updateHistory()
            {
               removeFromHistory();
               addToHistory();
            }
        
        template<int InFrequency, int OutFrequency>
          void ResampleConverter<InFrequency, OutFrequency>::removeFromHistory()
            {
               int toRemove = getDecimationFactor();
               while(toRemove > 0)
                 {
                    auto& oldestElement = _history.front();
                    if(oldestElement.numberOfZeros > 0)
                      {
                         if(oldestElement.numberOfZeros >= toRemove) 
                           {
                              oldestElement.numberOfZeros -= toRemove;
                              toRemove = 0;
                           }
                         else 
                           {
                              toRemove -= oldestElement.numberOfZeros;
                              oldestElement.numberOfZeros = 0;
                           }
                         
                      } 
                    else
                      {
                         _history.pop_front();
                         --toRemove;
                      }
                 }
            }
        
        template<int InFrequency, int OutFrequency>
          void ResampleConverter<InFrequency, OutFrequency>::addToHistory()
            {
               int toAdd = getDecimationFactor();
               while(toAdd > 0)
                 {
                    if(toAdd <= _zerosToNextInterpolatedSample)
                      {
                         _history.push_back(SampleWithZeros(0.0, toAdd-1));
                         _zerosToNextInterpolatedSample -= toAdd;
                         toAdd = 0;
                      }
                    else
                      {
                         if(nextSampleExists())
                           {
                              double sample = getNextSample();
                              _history.push_back(SampleWithZeros(sample, _zerosToNextInterpolatedSample));
                              toAdd -= (_zerosToNextInterpolatedSample + 1);
                              _zerosToNextInterpolatedSample = getInterpolationFactor() - 1;
                           } else {
                              toAdd = 0;
                           }
                      }
                 }
            }
        
        template<int InFrequency, int OutFrequency>
          double ResampleConverter<InFrequency, OutFrequency>::calculateInterpolatedSample() const
          {
             double sample = 0.0;
             int idxSampleWithZeros = 0;
             for(int i = 0; i < _numTaps; ++i) 
               {
                  auto& sampleWithZeros = _history.getSample(idxSampleWithZeros++);
                  i += sampleWithZeros.numberOfZeros;
                  if(i < _numTaps) 
                    {
                       sample += sampleWithZeros.sample * getInterpolationFactor() * (FilterType::taps.at(i));
                    }
               }
             return sample;
          }
        
        template<int InFrequency, int OutFrequency>
          double ResampleConverter<InFrequency, OutFrequency>::getNextSample()
          {
             if(_currentSample >= _inputBufferAsDouble.size())
               {
                  _src->read(&_inputBuffer, _inputArray.size());
                  _currentSample = 0;
               }
             return _inputBufferAsDouble[_currentSample++];
          }
        
        template<int InFrequency, int OutFrequency>
          bool ResampleConverter<InFrequency, OutFrequency>::nextSampleExists()
            {
               return _currentSample < _inputBufferAsDouble.size() || !_src->isFinished();
            }*/
	
	const mods::utils::ConstFraction& GenericResampleConverter::getResampleFraction() const
	  {
	     return _resampleFraction;
	  }
        
	int GenericResampleConverter::getInterpolationFactor() const
	  {
	     return getResampleFraction().getDenominator();
	  }
	
	GenericResampleConverter::SampleWithZeros::SampleWithZeros(double sample, int zeros)
	  : numberOfZeros(zeros),
          sample(sample)
            {
            }
	
	GenericResampleConverter::History::History(int numTaps)
	  : _v(numTaps)
	    {
	    }
        
        /*template<int InFrequency, int OutFrequency>
          typename ResampleConverter<InFrequency, OutFrequency>::SampleWithZeros& ResampleConverter<InFrequency, OutFrequency>::History::front()
          {
             if(_begin == _end)
               {
                  static SampleWithZeros zero(0.0, 0);
                  return zero;
               }
             
             return _array.at(_begin);
          }
        
        template<int InFrequency, int OutFrequency>
          void ResampleConverter<InFrequency, OutFrequency>::History::pop_front()
          {
             if(_begin == _end)
               {
                  return;
               }
             ++_begin;
             if(_begin == _array.size())
               {
                  _begin = 0;
               }
          }*/
        
	void GenericResampleConverter::History::push_back(const SampleWithZeros& sampleWithZeros)
	  {
	     _v[_end] = sampleWithZeros;
	     ++_end;
	     if(_end == _v.size())
	       {
		  _end = 0;
	       }
	  }
        
	bool GenericResampleConverter::History::isEmpty() const
          {
             return _begin == _end;
          }
        
        /*template<int InFrequency, int OutFrequency>
          const typename ResampleConverter<InFrequency, OutFrequency>::SampleWithZeros& ResampleConverter<InFrequency, OutFrequency>::History::getSample(size_t i) const
          {
             size_t idx = _begin + i;
             if(idx >= _array.size())
               {
                  idx -= _array.size();
               }
             return _array.at(idx);
          }*/
	
	GenericResampleConverter::ResampleSpec::ResampleSpec(int inputFrequency, int outputFrequency)
	  : _inputFrequency(inputFrequency),
	  _outputFrequency(outputFrequency)
	  {
	  }
	
	int GenericResampleConverter::ResampleSpec::getInputFrequency() const
	  {
	     return _inputFrequency;
	  }
	
	int GenericResampleConverter::ResampleSpec::getOutputFrequency() const
	  {
	     return _outputFrequency;
	  }
     } // namespace wav
} // namespace mods
