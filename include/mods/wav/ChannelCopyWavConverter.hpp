#ifndef MODS_WAV_CHANNELCOPYWAVCONVERTER_HPP
#define MODS_WAV_CHANNELCOPYWAVCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class ChannelCopyWavConverter : public WavConverter
          {
           public:
             ChannelCopyWavConverter() = default;
             ChannelCopyWavConverter(const ChannelCopyWavConverter&) = delete;
             ChannelCopyWavConverter(const ChannelCopyWavConverter&&) = delete;
             ChannelCopyWavConverter& operator=(const ChannelCopyWavConverter&) = delete;
             ChannelCopyWavConverter& operator=(const ChannelCopyWavConverter&&) = delete;
             ~ChannelCopyWavConverter() override = default;
             
             WavConverter::ptr getCopy();
             
             bool isFinished() const override;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_CHANNELCOPYWAVCONVERTER_HPP
