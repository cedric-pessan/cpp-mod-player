
#include "mods/converters/MultiplexerConverter.hpp"

#include <iostream>

namespace mods
{
   namespace converters
     {
        MultiplexerConverter::MultiplexerConverter(Converter::ptr left, Converter::ptr right)
          : _left(std::move(left)),
          _right(std::move(right)),
          _temp(allocateNewTempBuffer(0))
            {
            }
        
        auto MultiplexerConverter::isFinished() const -> bool
          {
             return _left->isFinished() && _right->isFinished();
          }
        
        void MultiplexerConverter::read(mods::utils::RWBuffer<u8>* buf, size_t len)
          {
             if((len % 4) != 0)
               {
                  std::cout << "TODO: wrong buffer length in MultiplexerWavConverter" << std::endl;
               }
             
             ensureTempBufferSize(len);
             
             size_t nbElems = (len/2) / sizeof(s16);
             auto leftBuf = _temp.slice<u8>(0, len/2);
             auto rightBuf = _temp.slice<u8>(len/2, len/2);
             
             _left->read(&leftBuf, len/2);
             _right->read(&rightBuf, len/2);
             
             auto outView = buf->slice<s16>(0, nbElems*2);
             auto leftView = leftBuf.slice<s16>(0, nbElems);
             auto rightView = rightBuf.slice<s16>(0, nbElems);
             
             for(size_t i=0; i<nbElems; ++i)
               {
                  outView[i*2] = leftView[i];
                  outView[i*2+1] = rightView[i];
               }
          }
        
        auto MultiplexerConverter::allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>
          {
             _tempVec.resize(len);
             u8* ptr = _tempVec.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
             return mods::utils::RWBuffer<u8>(std::move(buffer));
          }
        
        void MultiplexerConverter::ensureTempBufferSize(size_t len)
          {
             if(_temp.size() < len)
               {
                  _temp = allocateNewTempBuffer(len);
               }
          }
     } // namespace converters
} // namespace mods
