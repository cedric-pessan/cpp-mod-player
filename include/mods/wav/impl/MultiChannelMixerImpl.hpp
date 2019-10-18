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
                  InternalMultiChannelMixerSourceConverter& operator=(const InternalMultiChannelMixerSourceConverter&) = delete;
                  InternalMultiChannelMixerSourceConverter& operator=(InternalMultiChannelMixerSourceConverter&&) = delete;
                  ~InternalMultiChannelMixerSourceConverter() = default;
                  
                  bool isFinished(ChannelId outChannel) const;
                  void read(mods::utils::RWBuffer<u8>* buf, int len, ChannelId outChannel);
                  
                private:
                  mods::utils::RWBuffer<u8> allocateNewTempBuffer(std::vector<u8>* backendVec, size_t len);
                  void ensureChannelBuffersSizes(size_t len);
                  
                  void computeMixingCoefficients();
                  double mix(int idxOutBuffer, size_t idxSample) const;
                  
                  using UnconsumedBuffer = std::deque<double>;
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
