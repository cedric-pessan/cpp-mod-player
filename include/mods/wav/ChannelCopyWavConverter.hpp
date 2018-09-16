#ifndef MODS_WAV_CHANNELCOPYWAVCONVERTER_HPP
#define MODS_WAV_CHANNELCOPYWAVCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"
#include "mods/wav/impl/ChannelCopyWavConverterImpl.hpp"

namespace mods
{
   namespace wav
     {
        class ChannelCopyWavConverter : public impl::ChannelCopyWavConverterSlave
          {
           public:
             ChannelCopyWavConverter();
             
             ChannelCopyWavConverter(const ChannelCopyWavConverter&) = delete;
             ChannelCopyWavConverter(const ChannelCopyWavConverter&&) = delete;
             ChannelCopyWavConverter& operator=(const ChannelCopyWavConverter&) = delete;
             ChannelCopyWavConverter& operator=(const ChannelCopyWavConverter&&) = delete;
             ~ChannelCopyWavConverter() override = default;
             
             WavConverter::ptr getCopy();
             
           private:
             WavConverter::ptr _copy;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_CHANNELCOPYWAVCONVERTER_HPP
