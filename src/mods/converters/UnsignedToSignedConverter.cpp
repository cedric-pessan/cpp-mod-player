
#include "mods/converters/UnsignedToSignedConverter.hpp"

#include <iostream>

namespace mods
{
   namespace converters
     {
        template<typename T>
          UnsignedToSignedConverter<T>::UnsignedToSignedConverter(Converter::ptr src)
            : _src(std::move(src))
              {
              }
        
        template<typename T>
          auto UnsignedToSignedConverter<T>::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void UnsignedToSignedConverter<T>::read(mods::utils::RWBuffer<u8>* buf, size_t len)
            {
               if((len % sizeof(T)) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in UnsignedToSignedWavConverter" << std::endl;
                 }
               
               _src->read(buf, len);
               
               auto bufview = buf->slice<T>(0, len/sizeof(T));
               
               for(T& v : bufview)
                 {
                    u32 value = static_cast<u32>(v);
                    s32 svalue = static_cast<s32>(value);
                    v = static_cast<T>(svalue - getOffset());
                 }
            }
        
        template class UnsignedToSignedConverter<u8>;
     } // namespace converters
} // namespace mods
