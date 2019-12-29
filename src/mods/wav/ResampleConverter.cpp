
#include "mods/wav/ResampleConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<int InFrequency, int OutFrequency>
          ResampleConverter<InFrequency, OutFrequency>::ResampleConverter(WavConverter::ptr src)
            : _src(std::move(src)),
          _inputBuffer(initBuffer()),
          _inputBufferAsDouble(_inputBuffer.slice<double>(0, _numTaps))
            {
               _history.push_back(SampleWithZeros(0.0,_numTaps-1));
            }
        
        template<int InFrequency, int OutFrequency>
          mods::utils::RWBuffer<u8> ResampleConverter<InFrequency, OutFrequency>::initBuffer()
          {
             u8* ptr = _inputArray.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, _numTaps * sizeof(double), std::move(deleter));
             return mods::utils::RWBuffer<u8>(buffer);
          }
        
        template<int InFrequency, int OutFrequency>
          bool ResampleConverter<InFrequency, OutFrequency>::isFinished() const
          {
             return _src->isFinished() && _history.isEmpty();
          }
        
        template<int InFrequency, int OutFrequency>
          void ResampleConverter<InFrequency, OutFrequency>::read(mods::utils::RWBuffer<u8>* buf, int len)
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
        
        template<int InFrequency, int OutFrequency>
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
            }
        
        template<int InFrequency, int OutFrequency>
          ResampleConverter<InFrequency, OutFrequency>::ResampleConverter::SampleWithZeros::SampleWithZeros(double sample, int zeros)
            : numberOfZeros(zeros),
          sample(sample)
            {
            }
        
        template<int InFrequency, int OutFrequency>
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
          }
        
        template<int InFrequency, int OutFrequency>
          void ResampleConverter<InFrequency, OutFrequency>::History::push_back(const SampleWithZeros& sampleWithZeros)
            {
               _array.at(_end) = sampleWithZeros;
               ++_end;
               if(_end == _array.size())
                 {
                    _end = 0;
                 }
            }
        
        template<int InFrequency, int OutFrequency>
          bool ResampleConverter<InFrequency, OutFrequency>::History::isEmpty() const
          {
             return _begin == _end;
          }
        
        template<int InFrequency, int OutFrequency>
          const typename ResampleConverter<InFrequency, OutFrequency>::SampleWithZeros& ResampleConverter<InFrequency, OutFrequency>::History::getSample(size_t i) const
          {
             size_t idx = _begin + i;
             if(idx >= _array.size())
               {
                  idx -= _array.size();
               }
             return _array.at(idx);
          }
        
        template class ResampleConverter<22000, 44100>;
        template class ResampleConverter<8000, 44100>;
        template class ResampleConverter<48000, 44100>;
	template class ResampleConverter<10000, 44100>;
     } // namespace wav
} // namespace mods
