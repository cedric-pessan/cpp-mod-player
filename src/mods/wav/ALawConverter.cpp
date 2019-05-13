
#include "mods/wav/ALawConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename T>
          ALawConverter<T>::ALawConverter(WavConverter::ptr src)
            : _src(std::move(src))
              {
              }
        
        template<typename T>
          bool ALawConverter<T>::isFinished() const
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void ALawConverter<T>::read(mods::utils::RWBuffer<u8>* buf, int len)
            {
               if((len % sizeof(T)) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in ALawConverter" << std::endl;
                 }
               
               int nbElems = len / sizeof(T);
               int toReadLen = nbElems * sizeof(s8);
               
               _src->read(buf, toReadLen);
               
               auto inView = buf->slice<s8>(0, nbElems);
               auto outView = buf->slice<T>(0, nbElems);
               
               for(int i = nbElems-1; i>=0; --i)
                 {
                    s8 value = inView[i];
                    aLawTransform(&outView[i], value);
                 }
            }
        
        template<typename T>
          template<typename T2>
          typename std::enable_if<std::is_same<T2, T>::value && std::is_floating_point<T2>::value>::type ALawConverter<T>::aLawTransform(T2* out, s8 value) const
          {
             *out = value / 128.0;
          }
        
        template class ALawConverter<double>;
     } // namespace wav
} // namespace mods
