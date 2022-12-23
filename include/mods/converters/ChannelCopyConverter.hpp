#ifndef MODS_CONVERTERS_CHANNELCOPYCONVERTER_HPP
#define MODS_CONVERTERS_CHANNELCOPYCONVERTER_HPP

#include "mods/converters/impl/ChannelCopyConverterImpl.hpp"

namespace mods
{
   namespace converters
     {
        template<typename T>
          class ChannelCopyConverter : public impl::ChannelCopyConverterSlave<T>
          {
           public:
             explicit ChannelCopyConverter(typename Converter<T>::ptr src);
             
             ChannelCopyConverter() = delete;
             ChannelCopyConverter(const ChannelCopyConverter&) = delete;
             ChannelCopyConverter(ChannelCopyConverter&&) = delete;
             auto operator=(const ChannelCopyConverter&) -> ChannelCopyConverter& = delete;
             auto operator=(ChannelCopyConverter&&) -> ChannelCopyConverter& = delete;
             ~ChannelCopyConverter() override = default;
             
             auto getCopy() -> typename Converter<T>::ptr;
             
           private:
             typename Converter<T>::ptr _copy;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_CHANNELCOPYCONVERTER_HPP
