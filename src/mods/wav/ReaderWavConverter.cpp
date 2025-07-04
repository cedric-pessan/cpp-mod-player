
#include "mods/utils/RBuffer.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/ReaderWavConverter.hpp"
#include "mods/wav/StatCollector.hpp"

#include <cstddef>
#include <utility>

namespace mods
{
   namespace wav
     {
        ReaderWavConverter::ReaderWavConverter(mods::utils::RBuffer<u8> buffer, u8 defaultValue, StatCollector* statCollector)
          : _buffer(std::move(buffer)),
          _it(_buffer.begin()),
          _end(_buffer.end()),
          _defaultValue(defaultValue),
          _statCollector(statCollector)
            {
            }
        
        auto ReaderWavConverter::isFinished() const -> bool
          {
             return _it >= _end;
          }
        
        void ReaderWavConverter::read(mods::utils::RWBuffer<u8>* buf)
          {
             int bytesRead = 0;
             size_t bytesWritten = 0;
             auto& out = *buf;
             while(bytesWritten < buf->size())
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
             _statCollector->inc(bytesRead);
          }
     } // namespace wav
} // namespace mods
