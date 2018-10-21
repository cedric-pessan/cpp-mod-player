
#include "mods/wav/MultiplexerWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        MultiplexerWavConverter::MultiplexerWavConverter(WavConverter::ptr left, WavConverter::ptr right)
          : _left(std::move(left)),
          _right(std::move(right)),
          _temp(allocateTempBuffer(0))
            {
            }
        
        bool MultiplexerWavConverter::isFinished() const
          {
             return _left->isFinished() && _right->isFinished();
          }
        
        void MultiplexerWavConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             if((len % 4) != 0)
               {
                  std::cout << "TODO: wrong buffer length in MultiplexerWavConverter" << std::endl;
               }
             
             allocateTempBuffer(len);
             
             int nbElems = (len/2) / sizeof(s16);
             auto leftBuf = _temp.slice<u8>(0, len/2);
             auto rightBuf = _temp.slice<u8>(len/2, len/2);
             
             _left->read(&leftBuf, len/2);
             _right->read(&rightBuf, len/2);
             
             auto outView = buf->slice<s16>(0, nbElems*2);
             auto leftView = leftBuf.slice<s16>(0, nbElems);
             auto rightView = rightBuf.slice<s16>(0, nbElems);
             
             for(int i=0; i<nbElems; ++i)
               {
                  outView[i*2] = leftView[i];
                  outView[i*2+1] = rightView[i];
               }
          }
        
        mods::utils::RWBuffer<u8> MultiplexerWavConverter::allocateTempBuffer(size_t len)
          {
             if(_temp.size() < len)
               {
                  _tempVec.resize(len);
                  u8* ptr = _tempVec.data();
                  auto deleter = std::make_unique<mods::utils::BufferBackend::EmptyDeleter>();
                  auto buffer = std::make_shared<mods::utils::BufferBackend>(ptr, len, std::move(deleter));
                  _temp = mods::utils::RWBuffer<u8>(buffer);
               }
             return _temp;
          }
     } // namespace wav
} // namespace mods