
#include "mods/wav/MultiplexerWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        MultiplexerWavConverter::MultiplexerWavConverter(WavConverter::ptr left, WavConverter::ptr right)
          : _left(std::move(left)),
          _right(std::move(right))
            {
            }
        
        bool MultiplexerWavConverter::isFinished() const
          {
             return _left->isFinished() && _right->isFinished();
          }
        
        void MultiplexerWavConverter::read(mods::utils::RWBuffer<u8>& buf, int len)
          {
             _left->read(buf, len);
             std::cout << "TODO: MultiplexerWavConverter::read(...)" << std::endl;
          }
     } // namespace wav
} // namespace mods