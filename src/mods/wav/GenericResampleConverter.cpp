
#include "mods/utils/integerFactorization.hpp"
#include "mods/wav/GenericResampleConverter.hpp"

#include <algorithm>

namespace mods
{
   namespace wav
     {
	GenericResampleConverter::GenericResampleConverter(WavConverter::ptr src, int inFrequency, int outFrequency)
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
	     
	     std::cout << "compute resampler from " << inFrequency << "Hz to " << outFrequency << "Hz" << std::endl;
	     
	     _resampleFraction = mods::utils::ConstFraction(inFrequency, outFrequency).reduce();
	     
	     double cutoffFrequency = std::min(inFrequency, outFrequency) / 2.0;
	     double sampleFrequency = static_cast<double>(inFrequency) * static_cast<double>(getInterpolationFactor());
             
             double transitionWidth = 0.0;
             int numTaps = 0;
             do
               {
                  transitionWidth += 50;
                  double delta = transitionWidth / sampleFrequency;
                  numTaps = 3.3 / delta;
//                  std::cout << "transitionWidth:" << transitionWidth << ", numTaps: " << numTaps << ", sampleFrequency: " << sampleFrequency << std::endl;
               }
             while(numTaps > 100);
             if((numTaps & 1) == 0) ++numTaps;
             numTaps += 1500;
	     
	     std::vector<mods::utils::Band> bands;
	     bands.emplace_back(0.0, cutoffFrequency, 1.0, 5.0, sampleFrequency);
	     bands.emplace_back(cutoffFrequency + 50/*transitionWidth*/, sampleFrequency / 2.0, 0.0, -40.0, sampleFrequency);
             
             std::cout << "sample frequency of filter:" << sampleFrequency << std::endl;
	     
	     _designer = std::make_unique</*mods::utils::FirFilterDesigner*/FakeDesigner>(/*bands*/ sampleFrequency, cutoffFrequency);
             _designer->displayProgress();
	     _designer->optimizeFilter(1401/*numTaps*/);
             /*std::cout << "resampler coefficients: " << _resampleFraction.getNumerator() << " / " << _resampleFraction.getDenominator() << std::endl;
             if(_resampleFraction.getNumerator() > 10 || _resampleFraction.getDenominator() > 10)
               _designer->optimizeFilter(1401);
             else
               _designer->optimizeFilter(101);*/
	     
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
	     
	     _currentSample = taps.size();
	     
	     //_inputVec.resize((taps.size() * sizeof(double) / getInterpolationFactor())+sizeof(double));
             _inputVec.resize(((taps.size() / getInterpolationFactor())+1) * sizeof(double));
	     _inputBuffer = initBuffer();
	     _inputBufferAsDouble = _inputBuffer.slice<double>(0, _inputVec.size() / sizeof(double));
	  }
	
	std::vector<GenericResampleConverter::ResampleSpec> GenericResampleConverter::buildSpecs(int inFrequency, int outFrequency)
	  {
	     auto globalResampleFraction = mods::utils::ConstFraction(inFrequency, outFrequency).reduce();
	     auto globalInterpolationFactor = globalResampleFraction.getDenominator();
	     auto globalDecimationFactor = globalResampleFraction.getNumerator();
	     
	     auto interpolationFactors = mods::utils::integerFactorization(globalInterpolationFactor);
	     auto decimationFactors = mods::utils::integerFactorization(globalDecimationFactor);
	     
	     /*while(interpolationFactors.size() < decimationFactors.size())
	       {
		  interpolationFactors.push_back(1);
	       }
	     while(decimationFactors.size() < interpolationFactors.size())
	       {
		  decimationFactors.push_back(1);
	       }*/
	     
             for(auto f : interpolationFactors) std::cout << "interp: " << f << std::endl;
             for(auto f : decimationFactors) std::cout << "decim: " << f << std::endl;
             
	     std::sort(interpolationFactors.begin(), interpolationFactors.end());
	     std::sort(decimationFactors.begin(), decimationFactors.end());
	     
	     auto itInterpolation = interpolationFactors.rbegin();
	     auto itDecimation = decimationFactors.begin();
	     
	     std::vector<ResampleSpec> specs;
             specs.emplace_back(inFrequency, outFrequency); // bypass everything
             return specs;
	     
	     int currentFrequency = inFrequency;
	     
	     /*while((itInterpolation != interpolationFactors.rend()) ||
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
	       }*/
             //specs.emplace_back(inFrequency, outFrequency);
             while(itInterpolation != interpolationFactors.rend())
               {
                  int nextFrequency = currentFrequency * *itInterpolation;
                  specs.emplace_back(currentFrequency, nextFrequency);
                  currentFrequency = nextFrequency;
                  ++itInterpolation;
               }
             while(itDecimation != decimationFactors.end())
               {
                  int nextFrequency = currentFrequency / *itDecimation;
                  specs.emplace_back(currentFrequency, nextFrequency);
                  currentFrequency = nextFrequency;
                  ++itDecimation;
               }
	     
	     return specs;
	  }
        
	mods::utils::RWBuffer<u8> GenericResampleConverter::initBuffer()
          {
             u8* ptr = _inputVec.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, _inputVec.size(), std::move(deleter));
             return mods::utils::RWBuffer<u8>(buffer);
          }
        
	bool GenericResampleConverter::isFinished() const
          {
             return _src->isFinished() && _history->isEmpty();
          }
        
	void GenericResampleConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
	  {
	     if((len % sizeof(double)) != 0)
	       {
		  std::cout << "TODO: wrong buffer length in ResampleConverter" << std::endl;
	       }
	     
	     int nbElems = len / sizeof(double);
	     
	     auto outView = buf->slice<double>(0, nbElems);
	     
	     for(int i=0; i<nbElems; ++i)
	       {
		  outView[i] = getNextDecimatedSample();
	       }
	  }
        
	double GenericResampleConverter::getNextDecimatedSample()
	  {
	     updateHistory();
	     return calculateInterpolatedSample();
	  }
        
	void GenericResampleConverter::updateHistory()
	  {
	     removeFromHistory();
	     addToHistory();
	  }
	
	void GenericResampleConverter::removeFromHistory()
	  {
	     int toRemove = getDecimationFactor();
	     while(toRemove > 0)
	       {
		  auto& oldestElement = _history->front();
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
		       _history->pop_front();
		       --toRemove;
		    }
	       }
	  }
        
	void GenericResampleConverter::addToHistory()
	  {
	     int toAdd = getDecimationFactor();
	     while(toAdd > 0)
	       {
		  if(toAdd <= _zerosToNextInterpolatedSample)
		    {
                       bool merged = false;
                       if(!_history->isEmpty())
                         {
                            auto& latestElement = _history->back();
                            if(latestElement.sample == 0.0)
                              {
                                 latestElement.numberOfZeros += toAdd;
                                 merged = true;
                              }
                         }
                       if(!merged)
                         {
                            _history->push_back(SampleWithZeros(0.0, toAdd-1));
                         }
		       _zerosToNextInterpolatedSample -= toAdd;
		       toAdd = 0;
		    }
		  else
		    {
		       if(nextSampleExists())
			 {
			    double sample = getNextSample();
                            bool merged = false;
                            if(!_history->isEmpty())
                              {
                                 auto& latestElement = _history->back();
                                 if(latestElement.sample == 0.0)
                                   {
                                      latestElement.numberOfZeros += (1 + _zerosToNextInterpolatedSample);
                                      latestElement.sample = sample;
                                      merged = true;
                                   }
                              }
                            if(!merged)
                              {
                                 _history->push_back(SampleWithZeros(sample, _zerosToNextInterpolatedSample));
                              }
			    toAdd -= (_zerosToNextInterpolatedSample + 1);
			    _zerosToNextInterpolatedSample = getInterpolationFactor() - 1;
			 } else {
			    toAdd = 0;
			 }
		    }
	       }
	  }
        
	double GenericResampleConverter::calculateInterpolatedSample() const
          {
             double sample = 0.0;
             int idxSampleWithZeros = 0;
	     auto& taps = _designer->getTaps();
             for(size_t i = 0; i < taps.size(); ++i) 
               {
                  auto& sampleWithZeros = _history->getSample(idxSampleWithZeros++);
                  i += sampleWithZeros.numberOfZeros;
                  if(i < taps.size()) 
                    {
                       sample += sampleWithZeros.sample * getInterpolationFactor() * taps[i];
                    }
               }
             return sample;
          }
        
	double GenericResampleConverter::getNextSample()
          {
             if(_currentSample >= _inputBufferAsDouble.size())
               {
                  _src->read(&_inputBuffer, _inputVec.size());
                  _currentSample = 0;
               }
             return _inputBufferAsDouble[_currentSample++];
          }
        
	bool GenericResampleConverter::nextSampleExists()
	  {
	     return _currentSample < _inputBufferAsDouble.size() || !_src->isFinished();
	  }
	
	const mods::utils::ConstFraction& GenericResampleConverter::getResampleFraction() const
	  {
	     return _resampleFraction;
	  }
        
	int GenericResampleConverter::getInterpolationFactor() const
	  {
	     return getResampleFraction().getDenominator();
	  }
	
	int GenericResampleConverter::getDecimationFactor() const
	  {
	     return getResampleFraction().getNumerator();
	  }
	
	GenericResampleConverter::SampleWithZeros::SampleWithZeros(double sample, int zeros)
	  : numberOfZeros(zeros),
          sample(sample)
            {
            }
	
	GenericResampleConverter::History::History(int numTaps)
	  : _v(numTaps),
          _zeros(0.0, 0)
	    {
	    }
        
	GenericResampleConverter::SampleWithZeros& GenericResampleConverter::History::front()
          {
             if(_begin == _end)
               {
                  static SampleWithZeros zero(0.0, 0);
                  return zero;
               }
             
	     return _v[_begin];
          }
        
        GenericResampleConverter::SampleWithZeros& GenericResampleConverter::History::back()
          {
             if(_begin == _end)
               {
                  static SampleWithZeros zero(0.0, 0);
                  return zero;
               }
             if(_end == 0)
               {
                  return _v[_v.size()-1];
               }
             return _v[_end-1];
          }
        
	void GenericResampleConverter::History::pop_front()
          {
             if(_begin == _end)
               {
                  return;
               }
             ++_begin;
             if(_begin == _v.size())
               {
                  _begin = 0;
               }
          }
        
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
        
	const GenericResampleConverter::SampleWithZeros& GenericResampleConverter::History::getSample(size_t i)
          {
             size_t idx = _begin + i;
             if(idx >= _v.size())
               {
                  idx -= _v.size();
               }
             if(idx >= _end)
               {
                  _zeros.numberOfZeros = 1000000; // Large number for fast interpolation of zeros (zeros are skipped)
                  return _zeros;
               }
	     return _v[idx];
          }
	
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
