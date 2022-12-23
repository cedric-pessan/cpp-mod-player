
#include "mods/converters/MultiplexerConverter.hpp"

#include <iostream>

namespace mods
{
   namespace converters
     {
        template<typename T>
          MultiplexerConverter<T>::MultiplexerConverter(typename Converter<T>::ptr left, typename Converter<T>::ptr right)
            : _left(std::move(left)),
          _right(std::move(right)),
          _temp(allocateNewTempBuffer(0))
            {
            }
        
        template<typename T>
          auto MultiplexerConverter<T>::isFinished() const -> bool
          {
             return _left->isFinished() && _right->isFinished();
          }
        
        template<typename T>
          void MultiplexerConverter<T>::read(mods::utils::RWBuffer<T>* buf)
            {
               if((buf->size() % 2) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in MultiplexerWavConverter" << std::endl;
                 }
               
               ensureTempBufferSize(buf->size());
               
               size_t nbElems = buf->size();
               auto leftBuf = _temp.template slice<T>(0, nbElems/2);
               auto rightBuf = _temp.template slice<T>(nbElems/2, nbElems/2);
               
               _left->read(&leftBuf);
               _right->read(&rightBuf);
               
               auto& outView = *buf;
               
               for(size_t i=0; i<nbElems/2; ++i)
                 {
                    outView[i*2] = leftBuf[i];
                    outView[i*2+1] = rightBuf[i];
                 }
            }
        
        template<typename T>
          auto MultiplexerConverter<T>::allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<T>
          {
             _tempVec.resize(len * sizeof(T));
             u8* ptr = _tempVec.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, len * sizeof(T), std::move(deleter));
             return mods::utils::RWBuffer<T>(std::move(buffer));
          }
        
        template<typename T>
          void MultiplexerConverter<T>::ensureTempBufferSize(size_t len)
            {
               if(_temp.size() < len)
                 {
                    _temp = allocateNewTempBuffer(len);
                 }
            }
        
        template class MultiplexerConverter<s16>;
     } // namespace converters
} // namespace mods
