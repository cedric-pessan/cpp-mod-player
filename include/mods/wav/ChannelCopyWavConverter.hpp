#ifndef MODS_WAV_CHANNELCOPYWAVCONVERTER_HPP
#define MODS_WAV_CHANNELCOPYWAVCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class ChannelCopyWavConverterSlave : public WavConverter
          {
           protected:
             ChannelCopyWavConverterSlave(const WavConverter::sptr& src);
             
           public:
             ChannelCopyWavConverterSlave() = delete;
             ChannelCopyWavConverterSlave(const ChannelCopyWavConverterSlave&) = delete;
             ChannelCopyWavConverterSlave(const ChannelCopyWavConverterSlave&&) = delete;
             ChannelCopyWavConverterSlave& operator=(const ChannelCopyWavConverterSlave&) = delete;
             ChannelCopyWavConverterSlave& operator=(const ChannelCopyWavConverterSlave&&) = delete;
             ~ChannelCopyWavConverterSlave() override = default;
             
             bool isFinished() const override;
             
           protected:
             WavConverter::ptr buildSlave() const;
             
           private:
             WavConverter::sptr _src;
          };
        
        class ChannelCopyWavConverter : public ChannelCopyWavConverterSlave
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
