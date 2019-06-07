
#include "mods/wav/ShiftLeftConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename T>
          ShiftLeftConverter<T>::ShiftLeftConverter(WavConverter::ptr src, u32 shiftBy)
            : _src(std::move(src)),
          _shiftLeft(shiftBy),
          _shiftRight((sizeof(T)*8) - shiftBy),
          _maskRight((1u << shiftBy) -1)
              {
                 if(shiftBy >= sizeof(T)*8)
                   {
                      std::cout << "TODO: invalid shift in ShiftLeftConverter" << std::endl;
                   }
              }
        
        template<typename T>
          bool ShiftLeftConverter<T>::isFinished() const
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void ShiftLeftConverter<T>::read(mods::utils::RWBuffer<u8>* buf, int len)
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
                    v <<= _shiftLeft;
                    v |= ((v >> _shiftRight) & _maskRight);
                 }
            }
        
        template class ShiftLeftConverter<s32>;
     } // namespace wav
} // namespace mods