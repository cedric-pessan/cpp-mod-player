
#include "mods/wav/ResampleConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<int InFrequency, int OutFrequency>
          ResampleConverter<InFrequency, OutFrequency>::ResampleConverter(WavConverter::ptr src)
            : _src(std::move(src))
              {
                 _history.push_back(SampleWithZeros(/*0.0,*/_numTaps-1));
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
               
               //_src->read(buf, len);
               
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
                         _history.push_back(SampleWithZeros(/*0.0,*/ toAdd));
                         _zerosToNextInterpolatedSample -= toAdd;
                         toAdd = 0;
                      }
                    else
                      {
                         //double sample = getNextSample();
                         _history.push_back(SampleWithZeros(/*sample,*/ _zerosToNextInterpolatedSample));
                         toAdd -= (_zerosToNextInterpolatedSample + 1);
                         _zerosToNextInterpolatedSample = _interpolationFactor - 1;
                      }
                 }
            }
        
        template<int InFrequency, int OutFrequency>
          double ResampleConverter<InFrequency, OutFrequency>::calculateInterpolatedSample() const
          {
             std::cout << "TODO: ResampleConverter::calculateInterpolatedSample() const" << std::endl;
             return 0.0;
          }
        
        template<int InFrequency, int OutFrequency>
          ResampleConverter<InFrequency, OutFrequency>::ResampleConverter::SampleWithZeros::SampleWithZeros(/*double sample,*/ int zeros)
            : numberOfZeros(zeros)
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
        
        template class ResampleConverter<22000, 44100>;
     } // namespace wav
} // namespace mods
