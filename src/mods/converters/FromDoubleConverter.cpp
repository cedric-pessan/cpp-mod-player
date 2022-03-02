
#include "mods/converters/FromDoubleConverter.hpp"

#include <iostream>
#include <limits>

namespace mods
{
   namespace converters
     {
        template<typename T>
          FromDoubleConverter<T>::FromDoubleConverter(Converter::ptr src)
            : _src(std::move(src)),
          _temp(allocateNewTempBuffer(0))
            {
               if(sizeof(T) > sizeof(double))
                 {
                    std::cout << "TODO: very large data type to convert" << std::endl;
                 }
            }
        
        template<typename T>
          auto FromDoubleConverter<T>::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void FromDoubleConverter<T>::read(mods::utils::RWBuffer<u8>* buf, size_t len)
            {
               if((len % sizeof(T)) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in FromDoubleConverter" << std::endl;
                 }
               
               int nbElems = len / sizeof(T);
               size_t toReadLen = nbElems * sizeof(double);
               
               ensureTempBufferSize(toReadLen);
               
               _src->read(&_temp, toReadLen);
               
               auto inView = _temp.slice<double>(0, nbElems);
               auto outView = buf->slice<T>(0, nbElems);
               
               for(int i = 0; i<nbElems; ++i)
                 {
                    double value = inView[i];
                    if(value >= 1.0) {
                       value = 1.0;
                    }
                    if(value <= -1.0) {
                       value = -1.0;
                    }
                    if(value >= 0.0) {
                       value *= static_cast<double>(std::numeric_limits<T>::max());
                    } else {
                       value *= -static_cast<double>(std::numeric_limits<T>::min());
                    }
                    T intValue = static_cast<T>(value);
                    outView[i] = intValue;
               }
            }
        
        template<typename T>
          auto FromDoubleConverter<T>::allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>
            {
               _tempVec.resize(len);
               u8* ptr = _tempVec.data();
               auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
               auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
               return mods::utils::RWBuffer<u8>(std::move(buffer));
            }
        
        template<typename T>
          void FromDoubleConverter<T>::ensureTempBufferSize(size_t len)
            {
               if(_temp.size() < len)
                 {
                    _temp = allocateNewTempBuffer(len);
                 }
            }
        
        template class FromDoubleConverter<s16>;
     } // namespace converters
} // namespace mods
