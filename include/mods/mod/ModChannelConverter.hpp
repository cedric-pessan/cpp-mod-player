#ifndef MODS_MOD_MODCHANNELCONVERTER_HPP
#define MODS_MOD_MODCHANNELCONVERTER_HPP

#include "mods/converters/Converter.hpp"
#include "mods/mod/ChannelId.hpp"
#include "mods/utils/AmigaRLESample.hpp"

namespace mods
{
   namespace mod
     {
        class PatternListReader;
        
        class ModChannelConverter : public mods::converters::Converter<mods::utils::AmigaRLESample>
          {
           public:
             using RLESample = mods::utils::AmigaRLESample;
             
             explicit ModChannelConverter(PatternListReader* patternListReader, ChannelId channel);
             
             ModChannelConverter() = delete;
             ModChannelConverter(const ModChannelConverter&) = delete;
             ModChannelConverter(ModChannelConverter&&) = delete;
             auto operator=(const ModChannelConverter&) -> ModChannelConverter& = delete;
             auto operator=(ModChannelConverter&&) -> ModChannelConverter& = delete;
             ~ModChannelConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<RLESample>* buf) override;
             
           private:
             PatternListReader* _reader;
             ChannelId _channel;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_MODCHANNELCONVERTER_HPP