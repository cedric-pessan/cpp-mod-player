
#include "mods/wav/UnsignedToSignedWavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename T>
          UnsignedToSignedWavConverter<T>::UnsignedToSignedWavConverter(WavConverter::ptr src)
            : _src(std::move(src))
              {
              }
        
        template<typename T>
          bool UnsignedToSignedWavConverter<T>::isFinished() const
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void UnsignedToSignedWavConverter<T>::read(mods::utils::RWBuffer<u8>* buf, int len)
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
        
        template class UnsignedToSignedWavConverter<u8>;
     } // namespace wav
} // namespace mods
