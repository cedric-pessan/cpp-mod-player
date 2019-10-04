#ifndef MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP
#define MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             class InternalMultiChannelMixerSourceConverter
               {
                public:
                  using sptr = std::shared_ptr<InternalMultiChannelMixerSourceConverter>;
                  
                  InternalMultiChannelMixerSourceConverter() = default;
                  InternalMultiChannelMixerSourceConverter(const InternalMultiChannelMixerSourceConverter&) = delete;
                  InternalMultiChannelMixerSourceConverter(InternalMultiChannelMixerSourceConverter&&) = delete;
                  InternalMultiChannelMixerSourceConverter& operator=(const InternalMultiChannelMixerSourceConverter&) = delete;
                  InternalMultiChannelMixerSourceConverter& operator=(InternalMultiChannelMixerSourceConverter&&) = delete;
                  ~InternalMultiChannelMixerSourceConverter() = default;
                  
                  bool isFinished() const;
               };
             
             class MultiChannelMixerSlave : public WavConverter
               {
                protected:
                  MultiChannelMixerSlave(InternalMultiChannelMixerSourceConverter::sptr src);
                  
                public:
                  MultiChannelMixerSlave() = delete;
                  MultiChannelMixerSlave(const MultiChannelMixerSlave&) = delete;
                  MultiChannelMixerSlave(MultiChannelMixerSlave&&) = delete;
                  MultiChannelMixerSlave& operator=(const MultiChannelMixerSlave&) = delete;
                  MultiChannelMixerSlave& operator=(MultiChannelMixerSlave&&) = delete;
                  ~MultiChannelMixerSlave() override = default;
                  
                  bool isFinished() const override;
                  void read(mods::utils::RWBuffer<u8>* buf, int len) override;
                  
                protected:
                  WavConverter::ptr buildSlave() const;
                  
                private:
                  InternalMultiChannelMixerSourceConverter::sptr _src;
               };
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP
