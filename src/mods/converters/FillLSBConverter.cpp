
#include "mods/converters/FillLSBConverter.hpp"

#include <iostream>

namespace mods
{
   namespace converters
     {
        template<typename T>
          FillLSBConverter<T>::FillLSBConverter(typename Converter<T>::ptr src, u32 bitsToFill)
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
          void FillLSBConverter<T>::read(mods::utils::RWBuffer<T>* buf)
            {
               _src->read(buf);
               
               for(T& v : *buf)
                 {
                    u32 tmp = v;
                    tmp &= ~_mask;
                    tmp |= ((tmp >> _shift) & _mask);
                    v = tmp;
                 }
            }
        
        template class FillLSBConverter<s8>;
        template class FillLSBConverter<s16>;
        template class FillLSBConverter<s32>;
     } // namespace converters
} // namespace mods