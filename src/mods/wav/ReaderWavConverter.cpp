
#include "mods/wav/ReaderWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        template<int CH, int NBCHANNELS, int BITSPERCONTAINER>
          ReaderWavConverter<CH, NBCHANNELS, BITSPERCONTAINER>::ReaderWavConverter(mods::utils::RBuffer<u8> buffer, u8 defaultValue, StatCollector::sptr statCollector)
            : _buffer(std::move(buffer)),
          _it(_buffer.begin()),
          _end(_buffer.end()),
          _defaultValue(defaultValue),
          _statCollector(std::move(statCollector))
            {
               _it += CH * BYTESPERCONTAINER;
            }
        
        template<int CH, int NBCHANNELS, int BITSPERCONTAINER>
          bool ReaderWavConverter<CH, NBCHANNELS, BITSPERCONTAINER>::isFinished() const
          {
             return _it >= _end;
          }
        
        template<int CH, int NBCHANNELS, int BITSPERCONTAINER>
          void ReaderWavConverter<CH, NBCHANNELS, BITSPERCONTAINER>::read(mods::utils::RWBuffer<u8>* buf, int len)
            {
               size_t bytesRead = 0;
               auto& out = *buf;
               for(int i=0; i<len; i+=BYTESPERCONTAINER)
                 {
                    if(_it < _end) 
                      {
                         for(int b=0; b<BYTESPERCONTAINER; ++b)
                           {
                              if(_it < _end)
                                {
                                   out[i+b] = *_it;
                                   ++_it;
                                   ++bytesRead;
                                }
                              else
                                {
                                   out[i+b] = _defaultValue;
                                }
                           }
                         _it += (NBCHANNELS-1) * BYTESPERCONTAINER;
                      }
                    else
                      {
                         for(int b=0; b<BYTESPERCONTAINER; ++b)
                           {
                              out[i+b] = _defaultValue;
                           }
                      }
                 }
               _statCollector->inc(bytesRead);
            }
        
        template class ReaderWavConverter<0,1,8>;
        
        template class ReaderWavConverter<0,2,8>;
        template class ReaderWavConverter<1,2,8>;
        
        template class ReaderWavConverter<0,1,16>;
        
        template class ReaderWavConverter<0,2,16>;
        template class ReaderWavConverter<1,2,16>;
        
        template class ReaderWavConverter<0,1,24>;
        
        template class ReaderWavConverter<0,2,24>;
        template class ReaderWavConverter<1,2,24>;
        
        template class ReaderWavConverter<0,1,32>;
        
        template class ReaderWavConverter<0,2,32>;
        template class ReaderWavConverter<1,2,32>;
        
        template class ReaderWavConverter<0,1,64>;
        
        template class ReaderWavConverter<0,2,64>;
        template class ReaderWavConverter<1,2,64>;
     } // namespace wav
} // namespace mods
