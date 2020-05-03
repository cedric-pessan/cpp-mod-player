
#include "mods/wav/ReaderWavConverter.hpp"

namespace mods
{
   namespace wav
     {
        ReaderWavConverter::ReaderWavConverter(mods::utils::RBuffer<u8> buffer, u8 defaultValue, StatCollector::sptr statCollector, u32 channel, u32 nbChannels, u32 bitsPerContainer)
          : _buffer(std::move(buffer)),
          _it(_buffer.begin()),
          _end(_buffer.end()),
          _defaultValue(defaultValue),
          _statCollector(std::move(statCollector)),
          _bytesPerContainer(bitsPerContainer / BITS_IN_BYTE),
          _bytesBetweenContainers((nbChannels-1) * _bytesPerContainer)
            {
               _it += channel * _bytesPerContainer;
            }
        
        auto ReaderWavConverter::isFinished() const -> bool
          {
             return _it >= _end;
          }
        
        void ReaderWavConverter::read(mods::utils::RWBuffer<u8>* buf, size_t len)
          {
             int bytesRead = 0;
             size_t bytesWritten = 0;
             auto& out = *buf;
             while(bytesWritten < len)
               {
                  if(_it < _end) 
                    {
                       for(; _currentByte<_bytesPerContainer && bytesWritten < len; ++_currentByte)
                         {
                            if(_it < _end)
                              {
                                 out[bytesWritten++] = *_it;
                                 ++_it;
                                 ++bytesRead;
                              }
                            else
                              {
                                 out[bytesWritten++] = _defaultValue;
                              }
                         }
                       if(_currentByte == _bytesPerContainer) 
                         {
                            _it += _bytesBetweenContainers;
                            _currentByte = 0;
                         }
                    }
                  else
                    {
                       for(size_t b=0; b<_bytesPerContainer && bytesWritten < len; ++b)
                         {
                            out[bytesWritten++] = _defaultValue;
                         }
                    }
               }
             _statCollector->inc(bytesRead);
          }
     } // namespace wav
} // namespace mods
