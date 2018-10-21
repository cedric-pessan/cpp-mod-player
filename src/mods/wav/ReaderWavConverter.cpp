
#include "mods/wav/ReaderWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        ReaderWavConverter::ReaderWavConverter(mods::utils::RBuffer<u8> buffer, u8 defaultValue)
          : _buffer(std::move(buffer)),
          _it(_buffer.begin()),
          _end(_buffer.end()),
          _defaultValue(defaultValue)
            {
            }
        
        bool ReaderWavConverter::isFinished() const
          {
             return _it == _end;
          }
        
        void ReaderWavConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             auto& out = *buf;
             for(int i=0; i<len; ++i)
               {
                  if(_it != _end) 
                    {
                       out[i] = *_it;
                       ++_it;
                    }
                  else
                    {
                       out[i] = _defaultValue;
                    }
               }
          }
     } // namespace wav
} // namespace mods
