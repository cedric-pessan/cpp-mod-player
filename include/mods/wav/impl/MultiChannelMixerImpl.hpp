#ifndef MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP
#define MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP

#include "mods/converters/Converter.hpp"
#include "mods/utils/DynamicRingBuffer.hpp"

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
                  
                private:
                  using Converter = mods::converters::Converter<double>;
                  
                public:
                  InternalMultiChannelMixerSourceConverter(std::vector<Converter::ptr> channels, u32 channelMask);
                  
                  InternalMultiChannelMixerSourceConverter() = delete;
                  InternalMultiChannelMixerSourceConverter(const InternalMultiChannelMixerSourceConverter&) = delete;
                  InternalMultiChannelMixerSourceConverter(InternalMultiChannelMixerSourceConverter&&) = delete;
                  auto operator=(const InternalMultiChannelMixerSourceConverter&) -> InternalMultiChannelMixerSourceConverter& = delete;
                  auto operator=(InternalMultiChannelMixerSourceConverter&&) -> InternalMultiChannelMixerSourceConverter& = delete;
                  ~InternalMultiChannelMixerSourceConverter() = default;
                  
                  auto isFinished(ChannelId outChannel) const -> bool;
                  void read(mods::utils::RWBuffer<double>* buf, ChannelId outChannel);
                  
                private:
                  static auto allocateNewTempBuffer(std::vector<u8>* backendVec, size_t len) -> mods::utils::RWBuffer<double>;
                  void ensureChannelBuffersSizes(size_t len);
                  
                  void computeMixingCoefficients(u32 channelMask);
                  auto mix(int idxOutBuffer, size_t idxSample) const -> double;
                  
                  using UnconsumedBuffer = mods::utils::DynamicRingBuffer<double>;
                  std::array<UnconsumedBuffer,2> _unconsumedBuffers;
                  
                  std::vector<Converter::ptr> _channels;
                  std::vector<std::vector<u8>> _channelsVec;
                  std::vector<mods::utils::RWBuffer<double>> _channelsBuffers;
                  std::array<std::vector<double>, 2> _coefficients;
               };
             
             class MultiChannelMixerBase : public mods::converters::Converter<double>
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
                  void read(mods::utils::RWBuffer<double>* buf) override;
                  
                protected:
                  auto buildRightChannel() const -> Converter<double>::ptr;
                  
                private:
                  InternalMultiChannelMixerSourceConverter::sptr _src;
                  ChannelId _channel;
               };
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_MULTICHANNELMIXERIMPL_HPP
