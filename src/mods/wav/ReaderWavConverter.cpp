
#include "mods/wav/ReaderWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        template<int CH, int NBCHANNELS>
          ReaderWavConverter<CH, NBCHANNELS>::ReaderWavConverter(mods::utils::RBuffer<u8> buffer, u8 defaultValue)
            : _buffer(std::move(buffer)),
          _it(_buffer.begin()),
          _end(_buffer.end()),
          _defaultValue(defaultValue)
            {
               _it += CH;
            }
        
        template<int CH, int NBCHANNELS>
          bool ReaderWavConverter<CH, NBCHANNELS>::isFinished() const
          {
             return _it == _end;
          }
        
        template<int CH, int NBCHANNELS>
          void ReaderWavConverter<CH, NBCHANNELS>::read(mods::utils::RWBuffer<u8>* buf, int len)
            {
               auto& out = *buf;
               for(int i=0; i<len; ++i)
                 {
                    if(_it != _end) 
                      {
                         out[i] = *_it;
                         _it += NBCHANNELS;
                      }
                    else
                      {
                         out[i] = _defaultValue;
                      }
                 }
            }
        
        template class ReaderWavConverter<0,1>;
        template class ReaderWavConverter<0,2>;
        template class ReaderWavConverter<1,2>;
     } // namespace wav
} // namespace mods
