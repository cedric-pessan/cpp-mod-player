
#include "mods/wav/ResampleConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        template<typename PARAMETERS>
          ResampleConverter<PARAMETERS>::ResampleConverter(WavConverter::ptr src, PARAMETERS resampleParameters)
            : _src(std::move(src)),
          _resampleParameters(std::move(resampleParameters)),
          _inputVec(((_resampleParameters.getNumTaps() / _resampleParameters.getInterpolationFactor())+1) * sizeof(double)),
          _inputBuffer(initBuffer()),
          _inputBufferAsDouble(_inputBuffer.slice<double>(0, _inputVec.size() / sizeof(double))),
          _currentSample(_resampleParameters.getNumTaps()),
          _history(_resampleParameters.getNumTaps())
            {
               using impl::SampleWithZeros;
               _history.push_back(SampleWithZeros(0.0, _resampleParameters.getNumTaps()-1));
            }
        
        template<typename PARAMETERS>
          mods::utils::RWBuffer<u8> ResampleConverter<PARAMETERS>::initBuffer()
          {
             u8* ptr = _inputVec.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, _inputVec.size(), std::move(deleter));
             return mods::utils::RWBuffer<u8>(buffer);
          }
        
        template<typename PARAMETERS>
          bool ResampleConverter<PARAMETERS>::isFinished() const
          {
             return _src->isFinished() && _history.isEmpty();
          }
        
        template<typename PARAMETERS>
          void ResampleConverter<PARAMETERS>::read(mods::utils::RWBuffer<u8>* buf, int len)
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
        
        template<typename PARAMETERS>
          double ResampleConverter<PARAMETERS>::getNextDecimatedSample()
            {
               updateHistory();
               return calculateInterpolatedSample();
            }
        
        template<typename PARAMETERS>
          void ResampleConverter<PARAMETERS>::updateHistory()
            {
               removeFromHistory();
               addToHistory();
            }
        
        template<typename PARAMETERS>
          void ResampleConverter<PARAMETERS>::removeFromHistory()
            {
               int toRemove = _resampleParameters.getDecimationFactor();
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
        
        template<typename PARAMETERS>
          void ResampleConverter<PARAMETERS>::addToHistory()
            {
               using impl::SampleWithZeros;
               
               int toAdd = _resampleParameters.getDecimationFactor();
               while(toAdd > 0)
                 {
                    if(toAdd <= _zerosToNextInterpolatedSample)
                      {
                         bool merged = false;
                         if(!_history.isEmpty())
                           {
                              auto& latestElement = _history.back();
                              if(latestElement.sample == 0.0)
                                {
                                   latestElement.numberOfZeros += toAdd;
                                   merged = true;
                                }
                           }
                         if(!merged)
                           {
                              _history.push_back(SampleWithZeros(0.0, toAdd-1));
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
                              if(!_history.isEmpty())
                                {
                                   auto& latestElement = _history.back();
                                   if(latestElement.sample == 0.0)
                                     {
                                        latestElement.numberOfZeros += (1 + _zerosToNextInterpolatedSample);
                                        latestElement.sample = sample;
                                        merged = true;
                                     }
                                }
                              if(!merged)
                                {
                                   _history.push_back(SampleWithZeros(sample, _zerosToNextInterpolatedSample));
                                }
                              toAdd -= (_zerosToNextInterpolatedSample + 1);
                              _zerosToNextInterpolatedSample = _resampleParameters.getInterpolationFactor() - 1;
                           } else {
                              toAdd = 0;
                           }
                      }
                 }
            }
        
        template<typename PARAMETERS>
          double ResampleConverter<PARAMETERS>::calculateInterpolatedSample()
          {
             double sample = 0.0;
             int idxSampleWithZeros = 0;
             auto interpolationFactor = _resampleParameters.getInterpolationFactor();
             for(int i = 0; i < _resampleParameters.getNumTaps(); ++i) 
               {
                  auto& sampleWithZeros = _history.getSample(idxSampleWithZeros++);
                  i += sampleWithZeros.numberOfZeros;
                  if(i < _resampleParameters.getNumTaps()) 
                    {
                       sample += sampleWithZeros.sample * interpolationFactor * _resampleParameters.getTap(i);
                    }
               }
             return sample;
          }
        
        template<typename PARAMETERS>
          double ResampleConverter<PARAMETERS>::getNextSample()
          {
             if(_currentSample >= _inputBufferAsDouble.size())
               {
                  _src->read(&_inputBuffer, _inputVec.size());
                  _currentSample = 0;
               }
             return _inputBufferAsDouble[_currentSample++];
          }
        
        template<typename PARAMETERS>
          bool ResampleConverter<PARAMETERS>::nextSampleExists() const
            {
               return _currentSample < _inputBufferAsDouble.size() || !_src->isFinished();
            }
        
        namespace impl
          {
             SampleWithZeros::SampleWithZeros(double sample, int zeros)
               : numberOfZeros(zeros),
               sample(sample)
                 {
                 }
             
             History::History(int numTaps)
               : _v(numTaps),
               _zeros(0.0, 0)
                 {
                 }
             
             SampleWithZeros& History::front()
               {
                  if(_begin == _end)
                    {
                       static SampleWithZeros zero(0.0, 0);
                       return zero;
                    }
                  
                  return _v[_begin];
               }
             
             SampleWithZeros& History::back()
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
             
             void History::pop_front()
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
             
             void History::push_back(const SampleWithZeros& sampleWithZeros)
               {
                  _v[_end] = sampleWithZeros;
                  ++_end;
                  if(_end == _v.size())
                    {
                       _end = 0;
                    }
               }
             
             bool History::isEmpty() const
               {
                  return _begin == _end;
               }
             
             const SampleWithZeros& History::getSample(size_t i)
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
          } // namespace impl
        
        DynamicResampleParameters::DynamicResampleParameters(int inFrequency, int outFrequency)
          : _resampleFraction(mods::utils::ConstFraction(inFrequency, outFrequency).reduce()),
          _designer(static_cast<double>(inFrequency) * static_cast<double>(getInterpolationFactor()), // sampleFrequency
                    std::min(inFrequency, outFrequency) / 2.0) // cutoffFrequency
            {
            }
        
        int DynamicResampleParameters::getNumTaps() const
          {
             return _designer.getTaps().size();
          }
        
        const mods::utils::ConstFraction& DynamicResampleParameters::getResampleFraction() const
          {
             return _resampleFraction;
          }
        
        int DynamicResampleParameters::getInterpolationFactor() const
          {
             return getResampleFraction().getDenominator();
          }
        
        int DynamicResampleParameters::getDecimationFactor() const
          {
             return getResampleFraction().getNumerator();
          }
        
        double DynamicResampleParameters::getTap(size_t i) const
          {
             return _designer.getTaps()[i];
          }
        
        template class ResampleConverter<StaticResampleParameters<22000, 44100>>;
        template class ResampleConverter<StaticResampleParameters<8000, 44100>>;
        template class ResampleConverter<StaticResampleParameters<48000, 44100>>;
	template class ResampleConverter<StaticResampleParameters<10000, 44100>>;
        template class ResampleConverter<DynamicResampleParameters>;
     } // namespace wav
} // namespace mods
