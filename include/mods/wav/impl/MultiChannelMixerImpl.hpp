#ifndef MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP
#define MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP

#include "mods/wav/WavConverter.hpp"

#include <deque>

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             enum struct ChannelId
               {
                  LEFT,
                    RIGHT
               };
             
             class InternalMultiChannelMixerSourceConverter
               {
                public:
                  using sptr = std::shared_ptr<InternalMultiChannelMixerSourceConverter>;
                  
                  explicit InternalMultiChannelMixerSourceConverter(std::vector<WavConverter::ptr> src);
                  
                  InternalMultiChannelMixerSourceConverter() = delete;
                  InternalMultiChannelMixerSourceConverter(const InternalMultiChannelMixerSourceConverter&) = delete;
                  InternalMultiChannelMixerSourceConverter(InternalMultiChannelMixerSourceConverter&&) = delete;
                  InternalMultiChannelMixerSourceConverter& operator=(const InternalMultiChannelMixerSourceConverter&) = delete;
                  InternalMultiChannelMixerSourceConverter& operator=(InternalMultiChannelMixerSourceConverter&&) = delete;
                  ~InternalMultiChannelMixerSourceConverter() = default;
                  
                  bool isFinished(ChannelId outChannel) const;
                  void read(mods::utils::RWBuffer<u8>* buf, int len, ChannelId outChannel);
                  
                private:
                  mods::utils::RWBuffer<u8> allocateNewTempBuffer(std::vector<u8>* backendVec, size_t len);
                  void ensureChannelBuffersSizes(size_t len);
                  
                  double mix() const;
                  
                  using UnconsumedBuffer = std::deque<double>;
                  std::array<UnconsumedBuffer,2> _unconsumedBuffers;
                  
                  std::vector<WavConverter::ptr> _channels;
                  std::vector<std::vector<u8>> _channelsVec;
                  std::vector<mods::utils::RWBuffer<u8>> _channelsBuffers;
               };
             
             class MultiChannelMixerBase : public WavConverter
               {
                protected:
                  MultiChannelMixerBase(InternalMultiChannelMixerSourceConverter::sptr src, ChannelId channel);
                  
                public:
                  MultiChannelMixerBase() = delete;
                  MultiChannelMixerBase(const MultiChannelMixerBase&) = delete;
                  MultiChannelMixerBase(MultiChannelMixerBase&&) = delete;
                  MultiChannelMixerBase& operator=(const MultiChannelMixerBase&) = delete;
                  MultiChannelMixerBase& operator=(MultiChannelMixerBase&&) = delete;
                  ~MultiChannelMixerBase() override = default;
                  
                  bool isFinished() const override;
                  void read(mods::utils::RWBuffer<u8>* buf, int len) override;
                  
                protected:
                  WavConverter::ptr buildRightChannel() const;
                  
                private:
                  InternalMultiChannelMixerSourceConverter::sptr _src;
                  ChannelId _channel;
               };
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP
