
#include "mods/converters/Converter.hpp"
#include "mods/converters/FromDoubleConverter.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/RWBufferBackend.hpp"
#include "mods/utils/types.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>

namespace mods
{
   namespace converters
     {
        template<typename T>
          FromDoubleConverter<T>::FromDoubleConverter(Converter<double>::ptr src)
            : _src(std::move(src)),
          _temp(allocateNewTempBuffer(0))
            {
               if(sizeof(T) > sizeof(double))
                 {
                    std::cout << "TODO: very large data type to convert" << '\n';
                 }
            }
        
        template<typename T>
          auto FromDoubleConverter<T>::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void FromDoubleConverter<T>::read(mods::utils::RWBuffer<T>* buf)
            {
               const int nbElems = buf->size();
               
               ensureTempBufferSize(nbElems);
               
               auto inView = _temp.slice<double>(0, nbElems);
               auto& outView = *buf;
               
               _src->read(&inView);
               
               for(int i = 0; i<nbElems; ++i)
                 {
                    double value = inView[i];
                    value = std::min(value, 1.0);
                    value = std::max(value, -1.0);
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
          auto FromDoubleConverter<T>::allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<double>
            {
               _tempVec.resize(len * sizeof(double));
               u8* ptr = _tempVec.data();
               auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
               auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, len * sizeof(double), std::move(deleter));
               mods::utils::RWBuffer<u8> byteBuf(std::move(buffer));
               return byteBuf.slice<double>(0, len);
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
