
#include "mods/converters/CastConverter.hpp"
#include "mods/converters/Converter.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"

#include <iostream>

namespace mods
{
   namespace converters
     {
        template<typename TOut, typename TIn>
          CastConverter<TOut, TIn>::CastConverter(typename Converter<TIn>::ptr src)
            : _src(std::move(src))
              {
              }
        
        template<typename TOut, typename TIn>
          auto CastConverter<TOut, TIn>::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        template<typename TOut, typename TIn>
          void CastConverter<TOut, TIn>::read(mods::utils::RWBuffer<TOut>* buf)
            {
               if(((buf->size() * sizeof(TOut)) % sizeof(TIn)) != 0)
                 {
                    std::cout << "Wrong buffer length to cast a type of size " << sizeof(TIn) << " to a  type of size " << sizeof(TOut) << '\n';
                 }
               
               auto castedBuf = buf->template slice<TIn>(0, (buf->size() * sizeof(TOut)) / sizeof(TIn));
               _src->read(&castedBuf);
            }
        
        template class CastConverter<s16, u8>;
        template class CastConverter<s32, u8>;
        template class CastConverter<float, u8>;
        template class CastConverter<double, u8>;
     } // namespace converters
} // namespace mods
