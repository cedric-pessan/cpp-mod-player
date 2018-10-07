
#include "mods/wav/ReaderWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        ReaderWavConverter::ReaderWavConverter(mods::utils::RBuffer<u8> buffer)
          : _buffer(std::move(buffer)),
          _it(_buffer.begin()),
          _end(_buffer.end())
            {
            }
        
        bool ReaderWavConverter::isFinished() const
          {
             return _it == _end;
          }
     } // namespace wav
} // namespace mods
