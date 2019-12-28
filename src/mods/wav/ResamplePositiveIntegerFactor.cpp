
#include "mods/wav/ResamplePositiveIntegerFactor.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        template<typename T, int FACTOR>
          ResamplePositiveIntegerFactor<T, FACTOR>::ResamplePositiveIntegerFactor(WavConverter::ptr src)
            : _src(std::move(src))
              {
              }
        
        template<typename T, int FACTOR>
          bool ResamplePositiveIntegerFactor<T, FACTOR>::isFinished() const
          {
             return _src->isFinished();
          }
        
        template<typename T, int FACTOR>
          void ResamplePositiveIntegerFactor<T, FACTOR>::read(mods::utils::RWBuffer<u8>* buf, int len)
            {
               if((len % sizeof(T)*FACTOR) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in ResamplePositiveIntegerFactor" << std::endl;
                 }
               
               int nbElems = len / (sizeof(T)*FACTOR);
               int toReadLen = nbElems * sizeof(T);
               
               _src->read(buf, toReadLen);
               
               auto inView = buf->slice<T>(0, nbElems);
               auto outView = buf->slice<T>(0, nbElems*FACTOR);
               
               for(int i = nbElems-1; i>=0; --i)
                 {
                    T value = inView[i];
                    for(int j = FACTOR-1; j>=0; --j)
                    {
                       outView[i*FACTOR+j] = value;
                    }
               }
            }
        
        template class ResamplePositiveIntegerFactor<s16, 2>;
        template class ResamplePositiveIntegerFactor<double, 2>;
	
	template class ResamplePositiveIntegerFactor<s16, 4>;
        template class ResamplePositiveIntegerFactor<double, 4>;
     } // namespace wav
} // namespace mods
