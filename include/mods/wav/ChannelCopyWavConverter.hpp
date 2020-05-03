#ifndef MODS_WAV_CHANNELCOPYWAVCONVERTER_HPP
#define MODS_WAV_CHANNELCOPYWAVCONVERTER_HPP

#include "mods/wav/impl/ChannelCopyWavConverterImpl.hpp"

namespace mods
{
   namespace wav
     {
        class ChannelCopyWavConverter : public impl::ChannelCopyWavConverterSlave
          {
           public:
             explicit ChannelCopyWavConverter(WavConverter::ptr src);
             
             ChannelCopyWavConverter() = delete;
             ChannelCopyWavConverter(const ChannelCopyWavConverter&) = delete;
             ChannelCopyWavConverter(ChannelCopyWavConverter&&) = delete;
             auto operator=(const ChannelCopyWavConverter&) -> ChannelCopyWavConverter& = delete;
             auto operator=(ChannelCopyWavConverter&&) -> ChannelCopyWavConverter& = delete;
             ~ChannelCopyWavConverter() override = default;
             
             auto getCopy() -> WavConverter::ptr;
             
           private:
             WavConverter::ptr _copy;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_CHANNELCOPYWAVCONVERTER_HPP
