
#include "mods/converters/FillLSBConverter.hpp"

#include <iostream>

namespace mods
{
   namespace converters
     {
        template<typename T>
          FillLSBConverter<T>::FillLSBConverter(Converter::ptr src, u32 bitsToFill)
            : _src(std::move(src)),
          _shift(((sizeof(T) * BITS_IN_BYTE) - bitsToFill)-1),
          _mask((1U << bitsToFill) -1)
              {
                 if(bitsToFill >= sizeof(T)*BITS_IN_BYTE)
                   {
                      std::cout << "TODO: invalid number of bits to fill in FillLSBConverter" << std::endl;
                   }
              }
        
        template<typename T>
          auto FillLSBConverter<T>::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void FillLSBConverter<T>::read(mods::utils::RWBuffer<u8>* buf, size_t len)
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
     } // namespace converters
} // namespace mods