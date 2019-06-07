
#include "mods/wav/UnpackToTypeConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename T>
          UnpackToTypeConverter<T>::UnpackToTypeConverter(WavConverter::ptr src, size_t packSize)
            : _src(std::move(src)),
          _packSize(packSize)
            {
               if(_packSize > sizeof(T))
                 {
                    std::cout << "TODO: UnpackToTypeConverter: pack size is too large to fit in output type" << std::endl;
                 }
            }
        
        template<typename T>
          bool UnpackToTypeConverter<T>::isFinished() const
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void UnpackToTypeConverter<T>::read(mods::utils::RWBuffer<u8>* buf, int len)
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
               
               for(int i=0; i<nbElems; ++i)
                 {
                    T outValue = 0;
                    u8 inValue = 0;
                    
                    for(size_t j=0; j<_packSize; ++j)
                      {
                         inValue = inView[i*_packSize + j];
                         outValue |= ((u32)inValue << (8 * j));
                      }
                    
                    if((inValue & 0x80) != 0)
                      {
                         for(size_t j=_packSize; j<sizeof(T); ++j)
                           {
                              outValue |= (0xFF << (8 * j));
                           }
                      }
                    outView[i] = outValue;
                 }
            }
        
        template class UnpackToTypeConverter<s32>;
     } // namespace wav
} // namespace mods