#ifndef MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP
#define MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP

#include "mods/utils/DynamicRingBuffer.hpp"
#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             enum struct ChannelId
               {
                  Left,
                    Right
               };
             
             enum struct DepthPositions
               {
                  Front,
                    FrontCenter,
                    LowFrequency,
                    Back,
                    FrontSide,
                    BackCenter,
                    Side,
                    TopCenter,
                    TopFront,
                    TopFrontCenter,
                    TopBack,
                    TopBackCenter,
                    NbDepthPositions
               };
             
             enum struct ChannelTypes
               {
                  SPEAKER_FRONT_LEFT,
                    SPEAKER_FRONT_RIGHT,
                    SPEAKER_FRONT_CENTER,
                    SPEAKER_LOW_FREQUENCY,
                    SPEAKER_BACK_LEFT,
                    SPEAKER_BACK_RIGHT,
                    SPEAKER_FRONT_LEFT_OF_CENTER,
                    SPEAKER_FRONT_RIGHT_OF_CENTER,
                    SPEAKER_BACK_CENTER,
                    SPEAKER_SIDE_LEFT,
                    SPEAKER_SIDE_RIGHT,
                    SPEAKER_TOP_CENTER,
                    SPEAKER_TOP_FRONT_LEFT,
                    SPEAKER_TOP_FRONT_CENTER,
                    SPEAKER_TOP_FRONT_RIGHT,
                    SPEAKER_TOP_BACK_LEFT,
                    SPEAKER_TOP_BACK_CENTER,
                    SPEAKER_TOP_BACK_RIGHT,
                    NbChannelTypes
               };
             
             class InternalMultiChannelMixerSourceConverter
               {
                public:
                  using sptr = std::shared_ptr<InternalMultiChannelMixerSourceConverter>;
                  
                  InternalMultiChannelMixerSourceConverter(std::vector<WavConverter::ptr> channels, u32 channelMask);
                  
                  InternalMultiChannelMixerSourceConverter() = delete;
                  InternalMultiChannelMixerSourceConverter(const InternalMultiChannelMixerSourceConverter&) = delete;
                  InternalMultiChannelMixerSourceConverter(InternalMultiChannelMixerSourceConverter&&) = delete;
                  auto operator=(const InternalMultiChannelMixerSourceConverter&) -> InternalMultiChannelMixerSourceConverter& = delete;
                  auto operator=(InternalMultiChannelMixerSourceConverter&&) -> InternalMultiChannelMixerSourceConverter& = delete;
                  ~InternalMultiChannelMixerSourceConverter() = default;
                  
                  auto isFinished(ChannelId outChannel) const -> bool;
                  void read(mods::utils::RWBuffer<u8>* buf, int len, ChannelId outChannel);
                  
                private:
                  static auto allocateNewTempBuffer(std::vector<u8>* backendVec, size_t len) -> mods::utils::RWBuffer<u8>;
                  void ensureChannelBuffersSizes(size_t len);
                  
                  void computeMixingCoefficients();
                  auto mix(int idxOutBuffer, size_t idxSample) const -> double;
                  
                  using UnconsumedBuffer = mods::utils::DynamicRingBuffer<double>;
                  std::array<UnconsumedBuffer,2> _unconsumedBuffers;
                  
                  std::vector<WavConverter::ptr> _channels;
                  std::vector<std::vector<u8>> _channelsVec;
                  std::vector<mods::utils::RWBuffer<u8>> _channelsBuffers;
                  std::vector<mods::utils::RBuffer<double>> _channelsViews;
                  std::array<std::vector<double>, 2> _coefficients;
                  
                  u32 _channelMask;
               };
             
             class MultiChannelMixerBase : public WavConverter
               {
                protected:
                  MultiChannelMixerBase(InternalMultiChannelMixerSourceConverter::sptr src, ChannelId channel);
                  
                public:
                  MultiChannelMixerBase() = delete;
                  MultiChannelMixerBase(const MultiChannelMixerBase&) = delete;
                  MultiChannelMixerBase(MultiChannelMixerBase&&) = delete;
                  auto operator=(const MultiChannelMixerBase&) -> MultiChannelMixerBase& = delete;
                  auto operator=(MultiChannelMixerBase&&) -> MultiChannelMixerBase& = delete;
                  ~MultiChannelMixerBase() override = default;
                  
                  auto isFinished() const -> bool override;
                  void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
                  
                protected:
                  auto buildRightChannel() const -> WavConverter::ptr;
                  
                private:
                  InternalMultiChannelMixerSourceConverter::sptr _src;
                  ChannelId _channel;
               };
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP
