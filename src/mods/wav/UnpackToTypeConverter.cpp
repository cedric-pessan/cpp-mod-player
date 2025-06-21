
#include "mods/converters/Converter.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/UnpackToTypeConverter.hpp"

#include <cstddef>
#include <iostream>
#include <utility>

namespace mods
{
   namespace wav
     {
        template<typename T>
          UnpackToTypeConverter<T>::UnpackToTypeConverter(mods::converters::Converter<u8>::ptr src, size_t packSize)
            : _src(std::move(src)),
          _packSize(packSize)
            {
               if(_packSize > sizeof(T))
                 {
                    std::cout << "TODO: UnpackToTypeConverter: pack size is too large to fit in output type" << '\n';
                 }
            }
        
        template<typename T>
          auto UnpackToTypeConverter<T>::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void UnpackToTypeConverter<T>::read(mods::utils::RWBuffer<T>* buf)
            {
               const int nbElems = buf->size();
               const int toReadLen = nbElems * _packSize;
               
               auto castedBuf = buf->template slice<u8>(0, buf->size() * sizeof(T));
               auto inView = castedBuf.template slice<u8>(castedBuf.size() - toReadLen, toReadLen);
               auto& outView = *buf;
               
               _src->read(&inView);
               
               static constexpr u8 signMask = 0x80U;
               static constexpr u32 negativeSignExtensionByte = 0xFFU;
               static constexpr u32 maxShiftValueMask = 31U; 
               
               for(int i=0; i<nbElems; ++i)
                 {
                    u32 uValue = 0;
                    u8 inValue = 0;
                    
                    for(size_t j=0; j<_packSize; ++j)
                      {
                         inValue = inView[(i*_packSize) + j];
                         uValue |= (static_cast<u32>(inValue) << (BITS_IN_BYTE * j));
                      }
                    
                    if((inValue & signMask) != 0)
                      {
                         for(size_t j=_packSize; j<sizeof(T); ++j)
                           {
                              uValue |= (negativeSignExtensionByte << (BITS_IN_BYTE * j));
                           }
                      }
                    T outValue = uValue << (((sizeof(T) - _packSize)*BITS_IN_BYTE) & maxShiftValueMask);
                    outView[i] = outValue;
                 }
            }
        
        template class UnpackToTypeConverter<s32>;
     } // namespace wav
} // namespace mods
