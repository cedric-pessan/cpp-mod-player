
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
               _it += CH * BYTESPERSAMPLE;
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
               for(int i=0; i<len; i+=BYTESPERSAMPLE)
                 {
                    if(_it < _end) 
                      {
                         for(int b=0; b<BYTESPERSAMPLE; ++b)
                           {
                              if(_it < _end)
                                {
                                   out[i+b] = *_it;
                                   ++_it;
                                }
                              else
                                {
                                   out[i+b] = _defaultValue;
                                }
                           }
                         _it += (NBCHANNELS-1) * BYTESPERSAMPLE;
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
