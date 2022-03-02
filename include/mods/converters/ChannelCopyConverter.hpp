#ifndef MODS_CONVERTERS_CHANNELCOPYCONVERTER_HPP
#define MODS_CONVERTERS_CHANNELCOPYCONVERTER_HPP

#include "mods/converters/impl/ChannelCopyConverterImpl.hpp"

namespace mods
{
   namespace converters
     {
        class ChannelCopyConverter : public impl::ChannelCopyConverterSlave
          {
           public:
             explicit ChannelCopyConverter(Converter::ptr src);
             
             ChannelCopyConverter() = delete;
             ChannelCopyConverter(const ChannelCopyConverter&) = delete;
             ChannelCopyConverter(ChannelCopyConverter&&) = delete;
             auto operator=(const ChannelCopyConverter&) -> ChannelCopyConverter& = delete;
             auto operator=(ChannelCopyConverter&&) -> ChannelCopyConverter& = delete;
             ~ChannelCopyConverter() override = default;
             
             auto getCopy() -> Converter::ptr;
             
           private:
             Converter::ptr _copy;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_CHANNELCOPYCONVERTER_HPP
