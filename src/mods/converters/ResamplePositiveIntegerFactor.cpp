
#include "mods/converters/ResamplePositiveIntegerFactor.hpp"

#include <iostream>

namespace mods
{
   namespace converters
     {
        template<typename T, int FACTOR>
          ResamplePositiveIntegerFactor<T, FACTOR>::ResamplePositiveIntegerFactor(typename Converter<T>::ptr src)
            : _src(std::move(src))
              {
              }
        
        template<typename T, int FACTOR>
          auto ResamplePositiveIntegerFactor<T, FACTOR>::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        template<typename T, int FACTOR>
          void ResamplePositiveIntegerFactor<T, FACTOR>::read(mods::utils::RWBuffer<T>* buf)
            {
               int nbElems = buf->size() / FACTOR;
               
               auto inView = buf->template slice<T>(0, nbElems);
               
               _src->read(&inView);
               
               for(int i = nbElems-1; i>=0; --i)
                 {
                    T value = inView[i];
                    for(int j = FACTOR-1; j>=0; --j)
                    {
                       (*buf)[i*FACTOR+j] = value;
                    }
               }
            }
        
        template class ResamplePositiveIntegerFactor<s16, 2>;
        template class ResamplePositiveIntegerFactor<s32, 2>;
        template class ResamplePositiveIntegerFactor<double, 2>;
	
	template class ResamplePositiveIntegerFactor<s16, 4>;
        template class ResamplePositiveIntegerFactor<s32, 4>;
        template class ResamplePositiveIntegerFactor<double, 4>;
     } // namespace converters
} // namespace mods
