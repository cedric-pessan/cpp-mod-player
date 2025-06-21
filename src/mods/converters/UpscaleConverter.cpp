
#include "mods/converters/Converter.hpp"
#include "mods/converters/UpscaleConverter.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"

#include <iostream>

namespace mods
{
   namespace converters
     {
        template<typename TOut, typename TIn>
          UpscaleConverter<TOut, TIn>::UpscaleConverter(typename Converter<TIn>::ptr src)
            : _src(std::move(src))
              {
                 if(sizeof(TOut) > 2*sizeof(TIn))
                   {
                      std::cout << "TODO: out format too large" << '\n';
                   }
              }
        
        template<typename TOut, typename TIn>
          auto UpscaleConverter<TOut, TIn>::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        template<typename TOut, typename TIn>
          void UpscaleConverter<TOut, TIn>::read(mods::utils::RWBuffer<TOut>* buf)
            {
               const int nbElems = buf->size();
               
               auto inView = buf->template slice<TIn>(0, nbElems);
               
               _src->read(&inView);
               
               for(int i = nbElems-1; i>=0; --i)
                 {
                    const u32 inputValue = static_cast<u8>(inView[i]);
                    TOut value = (inputValue << shiftLeftValue()) | ((inputValue >> shiftRightValue()) & maskValue());
                    (*buf)[i] = value;
                 }
            }
        
        template class UpscaleConverter<s16, s8>;
     } // namespace converters
} // namespace mods
