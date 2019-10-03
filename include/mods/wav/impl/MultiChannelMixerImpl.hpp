#ifndef MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP
#define MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             class MultiChannelMixerSlave : public WavConverter
               {
                public:
                  MultiChannelMixerSlave() = default;
                  
                  //MultiChannelMixerSlave() = delete;
                  MultiChannelMixerSlave(const MultiChannelMixerSlave&) = delete;
                  MultiChannelMixerSlave(MultiChannelMixerSlave&&) = delete;
                  MultiChannelMixerSlave& operator=(const MultiChannelMixerSlave&) = delete;
                  MultiChannelMixerSlave& operator=(MultiChannelMixerSlave&&) = delete;
                  ~MultiChannelMixerSlave() override = default;
                  
                  bool isFinished() const override;
                  void read(mods::utils::RWBuffer<u8>* buf, int len) override;
                  
                protected:
                  WavConverter::ptr buildSlave() const;
               };
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP
