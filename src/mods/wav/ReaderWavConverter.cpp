
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
        
#define instantiateReaderWavConverter(bitsPerSample)     \
   template class ReaderWavConverter<0,1,bitsPerSample>; \
                                                         \
   template class ReaderWavConverter<0,2,bitsPerSample>; \
   template class ReaderWavConverter<1,2,bitsPerSample>; \
                                                         \
   template class ReaderWavConverter<0,4,bitsPerSample>; \
   template class ReaderWavConverter<1,4,bitsPerSample>; \
   template class ReaderWavConverter<2,4,bitsPerSample>; \
   template class ReaderWavConverter<3,4,bitsPerSample>; \
                                                         \
   template class ReaderWavConverter<0,6,bitsPerSample>; \
   template class ReaderWavConverter<1,6,bitsPerSample>; \
   template class ReaderWavConverter<2,6,bitsPerSample>; \
   template class ReaderWavConverter<3,6,bitsPerSample>; \
   template class ReaderWavConverter<4,6,bitsPerSample>; \
   template class ReaderWavConverter<5,6,bitsPerSample>; \
                                                         \
   template class ReaderWavConverter<0,8,bitsPerSample>; \
   template class ReaderWavConverter<1,8,bitsPerSample>; \
   template class ReaderWavConverter<2,8,bitsPerSample>; \
   template class ReaderWavConverter<3,8,bitsPerSample>; \
   template class ReaderWavConverter<4,8,bitsPerSample>; \
   template class ReaderWavConverter<5,8,bitsPerSample>; \
   template class ReaderWavConverter<6,8,bitsPerSample>; \
   template class ReaderWavConverter<7,8,bitsPerSample>  \
        
        instantiateReaderWavConverter(8);
        instantiateReaderWavConverter(16);
        instantiateReaderWavConverter(24);
        instantiateReaderWavConverter(32);
        instantiateReaderWavConverter(64);
        instantiateReaderWavConverter(256);
        instantiateReaderWavConverter(520);
     } // namespace wav
} // namespace mods
