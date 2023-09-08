
#include "mods/mod/ModChannelConverter.hpp"
#include "mods/mod/PatternListReader.hpp"

namespace mods
{
   namespace mod
     {
        ModChannelConverter::ModChannelConverter(std::shared_ptr<PatternListReader> patternListReader, ChannelId channel)
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
