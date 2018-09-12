
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
     } // namespace wav
} // namespace mods