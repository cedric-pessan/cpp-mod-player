
#include "mods/utils/integerFactorization.hpp"
#include "mods/wav/GenericResampleConverter.hpp"

#include <algorithm>

namespace mods
{
   namespace wav
     {
	GenericResampleConverter::GenericResampleConverter(WavConverter::ptr src, int inFrequency, int outFrequency)
	  {
	     _resampleFraction = mods::utils::ConstFraction(inFrequency, outFrequency).reduce();
	     
	     double cutoffFrequency = std::min(inFrequency, outFrequency) / 2.0;
	     double sampleFrequency = static_cast<double>(inFrequency) * static_cast<double>(getInterpolationFactor());
	     
	     std::vector<mods::utils::Band> bands;
	     bands.emplace_back(0.0, cutoffFrequency, 1.0, 5.0, sampleFrequency);
	     bands.emplace_back(cutoffFrequency + 50, sampleFrequency / 2.0, 0.0, -40.0, sampleFrequency);
	     
	     _designer = std::make_unique<mods::utils::FirFilterDesigner>(/*bands*/ sampleFrequency, cutoffFrequency);
	     _designer->optimizeFilter();
	     
	     auto& taps = _designer->getTaps();
	     _history = std::make_unique<History>(taps.size());
	     _history->push_back(SampleWithZeros(0.0,taps.size()-1));
	     
             _src = std::move(src);
	     
	     _currentSample = taps.size();
	     
             _inputVec.resize(((taps.size() / getInterpolationFactor())+1) * sizeof(double));
	     _inputBuffer = initBuffer();
	     _inputBufferAsDouble = _inputBuffer.slice<double>(0, _inputVec.size() / sizeof(double));
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
	
     } // namespace wav
} // namespace mods
