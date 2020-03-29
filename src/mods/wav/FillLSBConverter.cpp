
#include "mods/wav/FillLSBConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        template<typename T>
          FillLSBConverter<T>::FillLSBConverter(WavConverter::ptr src, u32 bitsToFill)
            : _src(std::move(src)),
          _shift(((sizeof(T) * 8) - bitsToFill)-1),
          _mask((1u << bitsToFill) -1)
              {
                 if(bitsToFill >= sizeof(T)*8)
                   {
                      std::cout << "TODO: invalid number of bits to fill in FillLSBConverter" << std::endl;
                   }
              }
        
        template<typename T>
          bool FillLSBConverter<T>::isFinished() const
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void FillLSBConverter<T>::read(mods::utils::RWBuffer<u8>* buf, int len)
            {
               if((len % sizeof(T)) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in ShiftLeftConverter" << std::endl;
                 }
               
               _src->read(buf, len);
               
               int nbElems = len / sizeof(T);
               auto bufView = buf->slice<T>(0, nbElems);
               
               for(T& v : bufView)
                 {
                    u32 tmp = v;
                    tmp &= ~_mask;
                    tmp |= ((tmp >> _shift) & _mask);
                    v = tmp;
                 }
            }
        
        template class FillLSBConverter<s16>;
        template class FillLSBConverter<s32>;
     } // namespace wav
} // namespace mods