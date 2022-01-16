#ifndef MODS_MOD_CHANNELSTATE_HPP
#define MODS_MOD_CHANNELSTATE_HPP

namespace mods
{
   namespace mod
     {
        class ChannelState
          {
           public:
             ChannelState() = default;
             ChannelState(const ChannelState&) = delete;
             ChannelState(ChannelState&&) = default;
             auto operator=(const ChannelState&) -> ChannelState& = delete;
             auto operator=(ChannelState&&) -> ChannelState& = delete;
             ~ChannelState() = default;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_CHANNELSTATE_HPP
