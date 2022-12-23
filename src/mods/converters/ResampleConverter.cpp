
#include "mods/converters/ResampleConverter.hpp"
#include "mods/converters/ResampleParameters.hpp"

namespace mods
{
   namespace converters
     {
        template<typename PARAMETERS>
          ResampleConverter<PARAMETERS>::ResampleConverter(typename Converter<double>::ptr src, PARAMETERS resampleParameters)
            : _resampleParameters(std::move(resampleParameters)),
          _history(_resampleParameters.getNumTaps()),
          _src(std::move(src)),
          _inputVec(((_resampleParameters.getNumTaps() / _resampleParameters.getInterpolationFactor())+1) * sizeof(double)),
          _inputBuffer(initBuffer()),
          _currentSample(_resampleParameters.getNumTaps())
            {
               using impl::SampleWithZeros;
               _history.push_back(SampleWithZeros(0.0, _resampleParameters.getNumTaps()-1));
            }
        
        template<typename PARAMETERS>
          auto ResampleConverter<PARAMETERS>::initBuffer() -> mods::utils::RWBuffer<double>
          {
             u8* ptr = _inputVec.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, _inputVec.size(), std::move(deleter));
             return mods::utils::RWBuffer<u8>(std::move(buffer)).slice<double>(0, _inputVec.size() / sizeof(double));
          }
        
        template<typename PARAMETERS>
          auto ResampleConverter<PARAMETERS>::isFinished() const -> bool
          {
             return _src->isFinished() && _history.isEmpty();
          }
        
        template<typename PARAMETERS>
          void ResampleConverter<PARAMETERS>::removeFromHistory()
            {
               int toRemove = _resampleParameters.getDecimationFactor();
               while(toRemove > 0)
                 {
                    auto& oldestElement = _history.front();
                    if(oldestElement.numberOfZeros() > 0)
                      {
                         if(oldestElement.numberOfZeros() >= toRemove) 
                           {
                              oldestElement.numberOfZeros() -= toRemove;
                              toRemove = 0;
                           }
                         else 
                           {
                              toRemove -= oldestElement.numberOfZeros();
                              oldestElement.numberOfZeros() = 0;
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
                              if(latestElement.sample() == 0.0)
                                {
                                   latestElement.numberOfZeros() += toAdd;
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
                                   if(latestElement.sample() == 0.0)
                                     {
                                        latestElement.numberOfZeros() += (1 + _zerosToNextInterpolatedSample);
                                        latestElement.sample() = sample;
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
          auto ResampleConverter<PARAMETERS>::getNextSample() -> double
          {
             if(_currentSample >= _inputBuffer.size())
               {
                  _src->read(&_inputBuffer);
                  _currentSample = 0;
               }
             return _inputBuffer[_currentSample++];
          }
        
        template<typename PARAMETERS>
          auto ResampleConverter<PARAMETERS>::nextSampleExists() const -> bool
            {
               return _currentSample < _inputBuffer.size() || !_src->isFinished();
            }
        
        template<typename PARAMETERS>
          auto ResampleConverter<PARAMETERS>::getResampleParameters() const -> const PARAMETERS&
          {
             return _resampleParameters;
          }
        
        template<typename PARAMETERS>
          auto ResampleConverter<PARAMETERS>::getHistory() -> impl::History&
            {
               return _history;
            }
        
        namespace impl
          {
             SampleWithZeros::SampleWithZeros(double sample, int zeros)
               : _numberOfZeros(zeros),
               _sample(sample)
                 {
                 }
             
             auto SampleWithZeros::numberOfZeros() -> int&
               {
                  return _numberOfZeros;
               }
             
             auto SampleWithZeros::numberOfZeros() const -> int
               {
                  return _numberOfZeros;
               }
             
             auto SampleWithZeros::sample() -> double&
               {
                  return _sample;
               }
             
             auto SampleWithZeros::sample() const -> double
               {
                  return _sample;
               }
             
             History::History(int numTaps)
               : _v(numTaps),
               _zeros(0.0, 0)
                 {
                 }
             
             auto History::front() -> SampleWithZeros&
               {
                  if(_begin == _end)
                    {
                       static SampleWithZeros zero(0.0, 0);
                       return zero;
                    }
                  
                  return _v[_begin];
               }
             
             auto History::back() -> SampleWithZeros&
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
                       _zeros.numberOfZeros() = _sizeOfZeroChunksReturnedWhenEmpty; // Large number for fast interpolation of zeros (zeros are skipped)
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
        
        template class ResampleConverter<StaticResampleParameters<StandardFrequency::_22000, StandardFrequency::_44100>>;
        template class ResampleConverter<StaticResampleParameters<StandardFrequency::_8000,  StandardFrequency::_44100>>;
        template class ResampleConverter<StaticResampleParameters<StandardFrequency::_48000, StandardFrequency::_44100>>;
	template class ResampleConverter<StaticResampleParameters<StandardFrequency::_10000, StandardFrequency::_44100>>;
        template class ResampleConverter<DynamicResampleParameters>;
     } // namespace converters
} // namespace mods
