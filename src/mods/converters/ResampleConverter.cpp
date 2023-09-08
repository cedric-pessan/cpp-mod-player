
#include "mods/converters/ResampleConverter.hpp"
#include "mods/converters/ResampleParameters.hpp"
#include "mods/utils/AmigaRLESample.hpp"

namespace mods
{
   namespace converters
     {
        template<typename PARAMETERS, typename T>
          ResampleConverter<PARAMETERS, T>::ResampleConverter(typename Converter<T>::ptr src, PARAMETERS resampleParameters)
            : _resampleParameters(std::move(resampleParameters)),
          _history(_resampleParameters.getNumTaps()),
          _src(std::move(src)),
          _inputVec(((_resampleParameters.getNumTaps() / _resampleParameters.getInterpolationFactor())+1) * sizeof(T)),
          _inputBuffer(initBuffer()),
          _currentSample(_resampleParameters.getNumTaps())
            {
               using impl::SampleWithZeros;
               _history.push_back(SampleWithZeros(0.0, _resampleParameters.getNumTaps()-1, false));
            }
        
        template<typename PARAMETERS, typename T>
          auto ResampleConverter<PARAMETERS, T>::initBuffer() -> mods::utils::RWBuffer<T>
          {
             u8* ptr = _inputVec.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, _inputVec.size(), std::move(deleter));
             return mods::utils::RWBuffer<u8>(std::move(buffer)).slice<T>(0, _inputVec.size() / sizeof(T));
          }
        
        template<typename PARAMETERS, typename T>
          auto ResampleConverter<PARAMETERS, T>::isFinished() const -> bool
          {
             return _src->isFinished() && _history.isEmpty();
          }
        
        template<typename PARAMETERS, typename T>
          void ResampleConverter<PARAMETERS, T>::removeFromHistory()
            {
               int toRemove = _resampleParameters.getDecimationFactor();
               while(toRemove > 0)
                 {
                    auto& oldestElement = _history.front();
                    if(oldestElement.getNumberOfZeros() > 0)
                      {
                         if(oldestElement.getNumberOfZeros() >= toRemove) 
                           {
                              oldestElement.setNumberOfZeros(oldestElement.getNumberOfZeros() - toRemove);
                              toRemove = 0;
                           }
                         else 
                           {
                              toRemove -= oldestElement.getNumberOfZeros();
                              oldestElement.setNumberOfZeros(0);
                           }
                         
                      } 
                    else
                      {
                         if(shouldDedup())
                           {
                              _history.popFrontAndUnmergeNextElement();
                           }
                         else
                           {
                              _history.pop_front();
                              
                           }
                         --toRemove;
                      }
                 }
            }
        
        template<typename PARAMETERS, typename T>
          auto ResampleConverter<PARAMETERS, T>::shouldDedup() const -> bool
          {
             return false;
          }
        
        template<>
          auto ResampleConverter<AmigaResampleParameters, mods::utils::AmigaRLESample>::shouldDedup() const -> bool
          {
             return true;
          }
        
        template<typename PARAMETERS, typename T>
          void ResampleConverter<PARAMETERS, T>::addToHistory()
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
                              if(latestElement.getSample() == 0.0)
                                {
                                   latestElement.setNumberOfZeros(latestElement.getNumberOfZeros() + toAdd);
                                   merged = true;
                                }
                           }
                         if(!merged)
                           {
                              if(shouldDedup())
                                {
                                   _history.tryToMergeLast2Elements();
                                }
                              _history.push_back(SampleWithZeros(0.0, toAdd-1, _resampleParameters.isFiltered()));
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
                                   if(latestElement.getSample() == 0.0)
                                     {
                                        latestElement.setNumberOfZeros(latestElement.getNumberOfZeros() + 1 + _zerosToNextInterpolatedSample);
                                        latestElement.setSample(sample);
                                        merged = true;
                                     }
                                }
                              if(!merged)
                                {
                                   if(shouldDedup())
                                     {
                                        _history.tryToMergeLast2Elements();
                                     }
                                   _history.push_back(SampleWithZeros(sample, _zerosToNextInterpolatedSample, _resampleParameters.isFiltered()));
                                }
                              toAdd -= (_zerosToNextInterpolatedSample + 1);
                              _zerosToNextInterpolatedSample = _resampleParameters.getInterpolationFactor() - 1;
                           } else {
                              toAdd = 0;
                           }
                      }
                 }
            }
        
        template<typename PARAMETERS, typename T>
          auto ResampleConverter<PARAMETERS, T>::getNextSample() -> double
          {
             if(_currentSample >= _inputBuffer.size())
               {
                  _src->read(&_inputBuffer);
                  _currentSample = 0;
               }
             return _inputBuffer[_currentSample++];
          }
        
        template<>
          auto ResampleConverter<AmigaResampleParameters, mods::utils::AmigaRLESample>::getNextSample() -> double
          {
             if(_resampleParameters.getAndConsumeCurrentSampleLength() > 0)
               {
                  return _resampleParameters.getCurrentSampleValue();
               }
             if(_currentSample >= _inputBuffer.size())
               {
                  _src->read(&_inputBuffer);
                  _currentSample = 0;
               }
             auto& rleSample = _inputBuffer[_currentSample++];
             auto value = rleSample.getValue();
             _resampleParameters.setFiltered(rleSample.isFiltered());
             _resampleParameters.setCurrentSampleValue(value);
             _resampleParameters.setCurrentSampleLength(rleSample.getLength()-1);
             return value;
          }
        
        template<typename PARAMETERS, typename T>
          auto ResampleConverter<PARAMETERS, T>::nextSampleExists() const -> bool
            {
               return currentSampleRemainingDuration() > 0 || _currentSample < _inputBuffer.size() || !_src->isFinished();
            }
        
        template<typename PARAMETERS, typename T>
          auto ResampleConverter<PARAMETERS, T>::currentSampleRemainingDuration() const -> size_t
          {
             return 0;
          }
        
        template<>
          auto ResampleConverter<AmigaResampleParameters, mods::utils::AmigaRLESample>::currentSampleRemainingDuration() const -> size_t
          {
             return _resampleParameters.getCurrentSampleLength();
          }
        
        template<typename PARAMETERS, typename T>
          auto ResampleConverter<PARAMETERS, T>::getResampleParameters() const -> const PARAMETERS&
          {
             return _resampleParameters;
          }
        
        template<typename PARAMETERS, typename T>
          auto ResampleConverter<PARAMETERS, T>::getHistory() -> impl::History&
            {
               return _history;
            }
        
        namespace impl
          {
             SampleWithZeros::SampleWithZeros(double sample, int zeros, bool filtered)
               : _numberOfZeros(zeros),
               _sample(sample),
               _filtered(filtered)
                 {
                 }
             
             void SampleWithZeros::setNumberOfZeros(int numberOfZeros)
               {
                  _numberOfZeros = numberOfZeros;
               }
             
             auto SampleWithZeros::getNumberOfZerosReference() -> int&
               {
                  return _numberOfZeros;
               }
             
             auto SampleWithZeros::getNumberOfZeros() const -> int
               {
                  return _numberOfZeros;
               }
             
             void SampleWithZeros::setSample(double sample)
               {
                  _sample = sample;
               }
             
             auto SampleWithZeros::getSampleReference() -> double&
               {
                  return _sample;
               }
             
             auto SampleWithZeros::getSample() const -> double
               {
                  return _sample;
               }
             
             auto SampleWithZeros::isFiltered() const -> bool
               {
                  return _filtered;
               }
             
             auto SampleWithZeros::isMergableWith(const SampleWithZeros& o) -> bool
               {
                  return _numberOfZeros == o._numberOfZeros &&
                    _sample == o._sample &&
                    _filtered == o._filtered;
               }
             
             void SampleWithZeros::setRepeatCount(int repeatCount)
               {
                  _repeatCount = repeatCount;
               }
             
             auto SampleWithZeros::getRepeatCount() const -> int
               {
                  return _repeatCount;
               }
             
             auto SampleWithZeros::getRepeatCountReference() -> int&
               {
                  return _repeatCount;
               }
             
             History::History(int numTaps)
               : _v(numTaps),
               _zeros(0.0, 0, false)
                 {
                 }
             
             auto History::front() -> SampleWithZeros&
               {
                  if(_begin == _end)
                    {
                       static SampleWithZeros zero(0.0, 0, false);
                       return zero;
                    }
                  
                  return _v[_begin];
               }
             
             auto History::back() -> SampleWithZeros&
               {
                  if(_begin == _end)
                    {
                       static SampleWithZeros zero(0.0, 0, false);
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
             
             void History::pop_back()
               {
                  if(_begin == _end)
                    {
                       return;
                    }
                  if(_end == 0)
                    {
                       _end = _v.size() - 1;
                    }
                  else
                    {
                       --_end;
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
                  if(_begin == _end) // resize
                    {
                       auto oldSize = _v.size();
                       std::vector<SampleWithZeros> newVec(oldSize * 2);
                       newVec[0] = _v[_begin];
                       ++_begin;
                       if(_begin == _v.size())
                         {
                            _begin = 0;
                         }
                       for(size_t i=1; i<oldSize; ++i) 
                         {
                            newVec[i] = getSample(i-1);
                         }
                       _v = std::move(newVec);
                       _begin = 0;
                       _end =  oldSize;
                    }
               }
             
             auto History::isEmpty() const -> bool
               {
                  return _begin == _end;
               }
             
             auto History::getSample(size_t i) -> SampleWithZeros&
               {
                  size_t idx = _begin + i;
                  if(idx >= _v.size())
                    {
                       idx -= _v.size();
                    }
                  if(idx >= _end)
                    {
                       _zeros.setNumberOfZeros(_sizeOfZeroChunksReturnedWhenEmpty); // Large number for fast interpolation of zeros (zeros are skipped)
                       return _zeros;
                    }
                  return _v[idx];
               }
             
             auto History::begin() -> History::iterator
               {
                  return { this, 0 };
               }
             
             auto History::end() -> History::iterator
               {
                  return { this, size() };
               }
             
             auto History::size() const -> size_t
               {
                  if(isEmpty())
                    {
                       return 0;
                    }
                  if(_begin < _end)
                    {
                       return _end - _begin;
                    }
                  size_t size = _v.size() - _begin;
                  size += _end;
                  return size;
               }
             
             void History::tryToMergeLast2Elements()
               {
                  if(size() >= 3) // after the merge we never want to be below 2 elements for the remove element algorithm to work
                    {
                       auto& e1 = getSample(size() - 2);
                       auto& e2 = getSample(size() - 1);
                       if(e1.isMergableWith(e2))
                         {
                            e1.setRepeatCount(e1.getRepeatCount()+1);
                            pop_back();
                         }
                    }
               }
             
             void History::popFrontAndUnmergeNextElement()
               {
                  if(size() == 1)
                    {
                       pop_front();
                       return;
                    }
                  auto& e2 = getSample(1);
                  if(e2.getRepeatCount() > 1)
                    {
                       auto& e1 = getSample(0);
                       e1 = e2;
                       e1.setRepeatCount(1);
                       e2.setRepeatCount(e2.getRepeatCount()-1);
                    }
                  else
                    {
                       pop_front();
                    }
               }
             
             History::Iterator::Iterator(History* history, size_t idx)
               : _history(*history),
               _idx(idx)
                 {
                 }
             
             auto History::Iterator::operator-(const Iterator& it) const -> History::difference_type
               {
                  return _idx - it._idx;
               }
             
             auto History::Iterator::operator*() const -> History::reference
               {
                  return _history.getSample(_idx);
               }
             
             auto History::Iterator::operator++() -> History::Iterator&
               {
                  ++_idx;
                  return *this;
               }
          } // namespace impl
        
        template class ResampleConverter<StaticResampleParameters<StandardFrequency::_22000, StandardFrequency::_44100>, double>;
        template class ResampleConverter<StaticResampleParameters<StandardFrequency::_8000,  StandardFrequency::_44100>, double>;
        template class ResampleConverter<StaticResampleParameters<StandardFrequency::_48000, StandardFrequency::_44100>, double>;
	template class ResampleConverter<StaticResampleParameters<StandardFrequency::_10000, StandardFrequency::_44100>, double>;
        template class ResampleConverter<DynamicResampleParameters, double>;
        template class ResampleConverter<AmigaResampleParameters, mods::utils::AmigaRLESample>;
     } // namespace converters
} // namespace mods
