
#include "mods/wav/ReaderWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        template<int CH, int NBCHANNELS, int BITSPERSAMPLE>
          ReaderWavConverter<CH, NBCHANNELS, BITSPERSAMPLE>::ReaderWavConverter(mods::utils::RBuffer<u8> buffer, u8 defaultValue)
            : _buffer(std::move(buffer)),
          _it(_buffer.begin()),
          _end(_buffer.end()),
          _defaultValue(defaultValue)
            {
               _it += CH * BITSPERSAMPLE/8;
            }
        
        template<int CH, int NBCHANNELS, int BITSPERSAMPLE>
          bool ReaderWavConverter<CH, NBCHANNELS, BITSPERSAMPLE>::isFinished() const
          {
             return _it >= _end;
          }
        
        template<int CH, int NBCHANNELS, int BITSPERSAMPLE>
          void ReaderWavConverter<CH, NBCHANNELS, BITSPERSAMPLE>::read(mods::utils::RWBuffer<u8>* buf, int len)
            {
               auto& out = *buf;
               for(int i=0; i<len; ++i)
                 {
                    if(_it < _end) 
                      {
                         out[i] = *_it;
                         _it += NBCHANNELS * BITSPERSAMPLE/8;
                      }
                    else
                      {
                         out[i] = _defaultValue;
                      }
                 }
            }
        
        template class ReaderWavConverter<0,1,8>;
        template class ReaderWavConverter<0,2,8>;
        template class ReaderWavConverter<1,2,8>;
     } // namespace wav
} // namespace mods
