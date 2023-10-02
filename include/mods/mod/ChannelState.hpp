#ifndef MODS_MOD_CHANNELSTATE_HPP
#define MODS_MOD_CHANNELSTATE_HPP

#include "mods/mod/NoEffect.hpp"
#include "mods/mod/Vibrato.hpp"
#include "mods/utils/AmigaRLESample.hpp"
#include "mods/utils/RBuffer.hpp"
#include "mods/utils/types.hpp"

#include <vector>

namespace mods
{
   namespace mod
     {
        class Note;
        class Instrument;
        
        class ChannelState
          {
           private:
             using RLESample = mods::utils::AmigaRLESample;
             
           public:
             explicit ChannelState(const std::vector<mods::utils::RBuffer<u8>>& sampleBuffers,
                                   const mods::utils::RBuffer<Instrument>& instruments);
             
             ChannelState() = delete;
             ChannelState(const ChannelState&) = delete;
             ChannelState(ChannelState&&) = default;
             auto operator=(const ChannelState&) -> ChannelState& = delete;
             auto operator=(ChannelState&&) -> ChannelState& = delete;
             ~ChannelState() = default;
             
             void prepareNextSample();
             auto getCurrentSampleLength() -> u32;
             auto readAndConsumeNextSample(u32 length) -> double;
             
             void updateChannelToNewLine(const mods::utils::RBuffer<Note>& note);
             void tick();
             
             auto hasSpeedDefined() const -> bool;
             auto getSpeed() const -> int;
             
           private:
             auto toDouble(s8 sample) -> double;
             auto getFineTuneFactor(int fineTune) -> double;
             void processNextSample(u16 sample);
             
             size_t _instrument = 0;
             size_t _currentSample = 0;
             size_t _currentRepeatSample = 0;
             u16 _period = 0;
             u16 _volume = 0;
             RLESample _currentValue;
             bool _speedSetOnLastLine = false;
             int _speed = 0;
             
             const std::vector<mods::utils::RBuffer<u8>>& _sampleBuffers;
             mods::utils::RBuffer<Instrument> _instruments;
             
             std::array<double, 16> _fineTuneFactors;
             
             std::unique_ptr<NoEffect> _noEffect;
             std::unique_ptr<Vibrato> _vibrato;
             Effect* _currentEffect = nullptr;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_CHANNELSTATE_HPP
