
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
             auto deleter = std::make_unique<mods::utils::BufferBackend::EmptyDeleter>();
             auto buffer = std::make_shared<mods::utils::BufferBackend>(ptr, _numTaps * sizeof(double), std::move(deleter));
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
             
               std::cout << "TODO: ResampleConverter::read(mods::utils::RWBuffer<u8>*, int)" << std::endl;
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
               int toRemove = _decimationFactor;
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
               int toAdd = _decimationFactor;
               while(toAdd > 0)
                 {
                    if(toAdd < _zerosToNextInterpolatedSample)
                      {
                         _history.push_back(SampleWithZeros(0.0, toAdd));
                         _zerosToNextInterpolatedSample -= toAdd;
                         toAdd = 0;
                      }
                    else
                      {
                         double sample = getNextSample();
                         _history.push_back(SampleWithZeros(sample, _zerosToNextInterpolatedSample));
                         toAdd -= (_zerosToNextInterpolatedSample + 1);
                         _zerosToNextInterpolatedSample = _interpolationFactor - 1;
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
                       sample += sampleWithZeros.sample * FilterType::taps[i];
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
                  std::cout << "TODO: ResampleConverter front() nothing on stack" << std::endl;
               }
             
             return _array[_begin];
          }
        
        template<int InFrequency, int OutFrequency>
          void ResampleConverter<InFrequency, OutFrequency>::History::pop_front()
          {
             if(_begin == _end)
               {
                  std::cout << "TODO: ResampleConverter::pop_front() nothing on stack" << std::endl;
               }
             ++_begin;
             if(_begin == _array.size()) _begin = 0;
          }
        
        template<int InFrequency, int OutFrequency>
          void ResampleConverter<InFrequency, OutFrequency>::History::push_back(const SampleWithZeros& sampleWithZeros)
            {
               _array[_end] = sampleWithZeros;
               ++_end;
               if(_end == _array.size()) _end = 0;
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
             return _array[idx];
          }
        
        template class ResampleConverter<22000, 44100>;
     } // namespace wav
} // namespace mods
