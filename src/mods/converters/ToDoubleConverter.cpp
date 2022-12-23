
#include "mods/converters/ToDoubleConverter.hpp"

#include <iostream>
#include <limits>

namespace mods
{
   namespace converters
     {
        template<typename T>
          ToDoubleConverter<T>::ToDoubleConverter(typename Converter<T>::ptr src)
            : _src(std::move(src))
              {
                 if(sizeof(T) > sizeof(double))
                   {
                      std::cout << "TODO: very large data type to convert" << std::endl;
                   }
              }
        
        template<typename T>
          auto ToDoubleConverter<T>::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void ToDoubleConverter<T>::read(mods::utils::RWBuffer<double>* buf)
            {
               size_t nbElems = buf->size();
               
               auto inView = buf->slice<T>(0, nbElems);
               auto& outView = *buf;
               
               _src->read(&inView);
               
               for(size_t i=0; i<nbElems; ++i)
                 {
                    size_t idx = nbElems - 1 - i;
                    double value = convert(inView[idx]);
                    outView[idx] = value;
                 }
            }
        
        template<>
          auto ToDoubleConverter<float>::convert(float in) -> double
            {
               return in;
            }
        
        template<typename T>
          auto ToDoubleConverter<T>::convert(T in) -> double
            {
               double value = in;
               if(value >= 0)
                 {
                    value /= static_cast<double>(std::numeric_limits<T>::max());
                 }
               else
                 {
                    value /= -static_cast<double>(std::numeric_limits<T>::min());
                 }
               return value;
            }
        
        template class ToDoubleConverter<s8>;
        template class ToDoubleConverter<s16>;
        template class ToDoubleConverter<s32>;
        template class ToDoubleConverter<float>;
     } // namespace converters
} // namespace mods
