#ifndef MODS_MOD_CHANNELSTATE_HPP
#define MODS_MOD_CHANNELSTATE_HPP

#include "mods/utils/RBuffer.hpp"
#include "mods/utils/types.hpp"

#include <vector>

namespace mods
{
   namespace mod
     {
        class Note;
        
        class ChannelState
          {
           public:
             ChannelState(const std::vector<mods::utils::RBuffer<u8>>& sampleBuffers);
             
             ChannelState() = delete;
             ChannelState(const ChannelState&) = delete;
             ChannelState(ChannelState&&) = default;
             auto operator=(const ChannelState&) -> ChannelState& = delete;
             auto operator=(ChannelState&&) -> ChannelState& = delete;
             ~ChannelState() = default;
             
             auto readNextSample() -> s16;
             
             void updateChannelToNewLine(const mods::utils::RBuffer<Note>& note);
             
           private:
             size_t _instrument = 0;
             size_t _currentSample = 0;
             
             const std::vector<mods::utils::RBuffer<u8>>& _sampleBuffers;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_CHANNELSTATE_HPP
