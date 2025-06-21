
#include "mods/mod/ChannelId.hpp"
#include "mods/mod/ModChannelConverter.hpp"
#include "mods/mod/PatternListReader.hpp"
#include "mods/utils/RWBuffer.hpp"

namespace mods
{
   namespace mod
     {
        ModChannelConverter::ModChannelConverter(PatternListReader* patternListReader, ChannelId channel)
          : _reader(patternListReader),
          _channel(channel)
            {
            }
        
        auto ModChannelConverter::isFinished() const -> bool
          {
             return _reader->isChannelFinished(_channel);
          }
        
        void ModChannelConverter::read(mods::utils::RWBuffer<RLESample>* buf)
          {
             _reader->readChannel(buf, _channel);
          }
     } // namespace mod
} // namespace mods
