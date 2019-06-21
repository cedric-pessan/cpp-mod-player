
#include "mods/wav/ToDoubleConverter.hpp"

#include <limits>

namespace mods
{
   namespace wav
     {
        template<typename T>
          ToDoubleConverter<T>::ToDoubleConverter(WavConverter::ptr src)
            : _src(std::move(src))
              {
                 if(sizeof(T) > sizeof(double))
                   {
                      std::cout << "TODO: very large data type to convert" << std::endl;
                   }
              }
        
        template<typename T>
          bool ToDoubleConverter<T>::isFinished() const
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void ToDoubleConverter<T>::read(mods::utils::RWBuffer<u8>* buf, int len)
            {
               if((len % sizeof(double)) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in ToDoubleConverter" << std::endl;
                 }
               
               int nbElems = len / sizeof(double);
               int toReadLen = nbElems * sizeof(T);
               
               _src->read(buf, toReadLen);
               
               auto inView = buf->slice<T>(0, nbElems);
               auto outView = buf->slice<double>(0, nbElems);
               
               for(int i = nbElems-1; i>=0; --i)
                 {
                    double value = inView[i];
                    if(value >= 0)
                      {
                         value /= static_cast<double>(std::numeric_limits<T>::max());
                      }
                    else
                      {
                         value /= -static_cast<double>(std::numeric_limits<T>::min());
                      }
                    outView[i] = value;
                 }
            }
        
        template class ToDoubleConverter<s8>;
        template class ToDoubleConverter<s16>;
        template class ToDoubleConverter<s32>;
     } // namespace wav
} // namespace mods
