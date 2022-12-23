
#include "mods/converters/UpscaleConverter.hpp"

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
                      std::cout << "TODO: out format too large" << std::endl;
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
               int nbElems = buf->size();
               
               auto inView = buf->template slice<TIn>(0, nbElems);
               
               _src->read(&inView);
               
               for(int i = nbElems-1; i>=0; --i)
                 {
                    u32 v = static_cast<u8>(inView[i]);
                    TOut value = (v << shiftLeftValue()) | ((v >> shiftRightValue()) & maskValue());
                    (*buf)[i] = value;
                 }
            }
        
        template class UpscaleConverter<s16, s8>;
     } // namespace converters
} // namespace mods
