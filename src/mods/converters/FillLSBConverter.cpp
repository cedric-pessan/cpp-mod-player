
#include "mods/converters/Converter.hpp"
#include "mods/converters/FillLSBConverter.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"

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
                      std::cout << "TODO: invalid number of bits to fill in FillLSBConverter" << '\n';
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
               
               for(T& value : *buf)
                 {
                    u32 tmp = static_cast<u32>(static_cast<s32>(value));
                    tmp &= ~_mask;
                    tmp |= ((tmp >> _shift) & _mask);
                    value = tmp;
                 }
            }
        
        template class FillLSBConverter<s8>;
        template class FillLSBConverter<s16>;
        template class FillLSBConverter<s32>;
     } // namespace converters
} // namespace mods