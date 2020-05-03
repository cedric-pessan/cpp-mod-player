#ifndef MODS_WAV_MULTICHANNELMIXER_HPP
#define MODS_WAV_MULTICHANNELMIXER_HPP

#include "mods/wav/impl/MultiChannelMixerImpl.hpp"

namespace mods
{
   namespace wav
     {
        class MultiChannelMixer : public impl::MultiChannelMixerBase
          {
           public:
             MultiChannelMixer(std::vector<WavConverter::ptr> channels, u32 channelMask);
             
             MultiChannelMixer() = delete;
             MultiChannelMixer(const MultiChannelMixer&) = delete;
             MultiChannelMixer(MultiChannelMixer&&) = delete;
             auto operator=(const MultiChannelMixer&) -> MultiChannelMixer& = delete;
             auto operator=(MultiChannelMixer&&) -> MultiChannelMixer& = delete;
             ~MultiChannelMixer() override = default;
             
             auto getRightChannel() -> WavConverter::ptr;
             
           private:
             WavConverter::ptr _right;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_MULTICHANNELMIXER_HPP
