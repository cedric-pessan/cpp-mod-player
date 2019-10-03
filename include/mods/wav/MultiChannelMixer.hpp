#ifndef MODS_WAV_MULTICHANNELMIXER_HPP
#define MODS_WAV_MULTICHANNELMIXER_HPP

#include "mods/wav/impl/MultiChannelMixerImpl.hpp"

namespace mods
{
   namespace wav
     {
        class MultiChannelMixer : public impl::MultiChannelMixerSlave
          {
           public:
             MultiChannelMixer();
             
             //MultiChannelMixer() = delete;
             MultiChannelMixer(const MultiChannelMixer&) = delete;
             MultiChannelMixer(MultiChannelMixer&&) = delete;
             MultiChannelMixer& operator=(const MultiChannelMixer&) = delete;
             MultiChannelMixer& operator=(MultiChannelMixer&&) = delete;
             ~MultiChannelMixer() override = default;
             
             WavConverter::ptr getRightChannel();
             
           private:
             WavConverter::ptr _right;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_MULTICHANNELMIXER_HPP
