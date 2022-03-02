
#include "mods/wav/UnpackToTypeConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        template<typename T>
          UnpackToTypeConverter<T>::UnpackToTypeConverter(Converter::ptr src, size_t packSize)
            : _src(std::move(src)),
          _packSize(packSize)
            {
               if(_packSize > sizeof(T))
                 {
                    std::cout << "TODO: UnpackToTypeConverter: pack size is too large to fit in output type" << std::endl;
                 }
            }
        
        template<typename T>
          auto UnpackToTypeConverter<T>::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void UnpackToTypeConverter<T>::read(mods::utils::RWBuffer<u8>* buf, size_t len)
            {
               if((len % sizeof(T)) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in UnpackToTypeConverter" << std::endl;
                 }
               
               int nbElems = len / sizeof(T);
               int toReadLen = nbElems * _packSize;
               
               auto inView = buf->slice<u8>(len - toReadLen, toReadLen);
               auto outView = buf->slice<T>(0, nbElems);
               
               _src->read(&inView, toReadLen);
               
               static constexpr u8 signMask = 0x80U;
               static constexpr u32 negativeSignExtensionByte = 0xFFU;
               static constexpr u32 maxShiftValueMask = 31U; 
               
               for(int i=0; i<nbElems; ++i)
                 {
                    u32 v = 0;
                    u8 inValue = 0;
                    
                    for(size_t j=0; j<_packSize; ++j)
                      {
                         inValue = inView[i*_packSize + j];
                         v |= (static_cast<u32>(inValue) << (BITS_IN_BYTE * j));
                      }
                    
                    if((inValue & signMask) != 0)
                      {
                         for(size_t j=_packSize; j<sizeof(T); ++j)
                           {
                              v |= (negativeSignExtensionByte << (BITS_IN_BYTE * j));
                           }
                      }
                    T outValue = v << (((sizeof(T) - _packSize)*BITS_IN_BYTE) & maxShiftValueMask);
                    outView[i] = outValue;
                 }
            }
        
        template class UnpackToTypeConverter<s32>;
     } // namespace wav
} // namespace mods
