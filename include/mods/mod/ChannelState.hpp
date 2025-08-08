#ifndef MODS_MOD_CHANNELSTATE_HPP
#define MODS_MOD_CHANNELSTATE_HPP

#include "mods/mod/Arpeggio.hpp"
#include "mods/mod/NoEffect.hpp"
#include "mods/mod/Retrigger.hpp"
#include "mods/mod/SlideDown.hpp"
#include "mods/mod/SlideToNote.hpp"
#include "mods/mod/SlideToNoteAndVolumeSlide.hpp"
#include "mods/mod/SlideUp.hpp"
#include "mods/mod/Vibrato.hpp"
#include "mods/mod/VibratoAndVolumeSlide.hpp"
#include "mods/mod/VolumeSlide.hpp"
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
             explicit ChannelState(const std::vector<mods::utils::RBuffer<s8>>& sampleBuffers,
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
             auto getSpeed() const -> u32;
             
             auto hasPatternJump() const -> bool;
             auto getPatternOfJumpTarget() const -> int;
             auto getLineOfJumpTarget() const -> int;
             
             auto isStartOfLoop() const -> bool;
             auto isEndOfLoop() const -> bool;
             auto getLoopLength() const -> u32;
             
           private:
             static auto toDouble(s8 sample) -> double;
             auto getFineTuneFactor(int fineTune) -> double;
             void processNextSample(s8 sample);
             void applyEffect(const mods::utils::RBuffer<Note>& note);
             
             void applyArpeggioEffect(const mods::utils::RBuffer<Note>& note);
             void applySlideUpEffect(const mods::utils::RBuffer<Note>& note);
             void applySlideDownEffect(const mods::utils::RBuffer<Note>& note);
             void applySlideToNoteEffect(const mods::utils::RBuffer<Note>& note);
             void applyVibratoEffect(const mods::utils::RBuffer<Note>& note);
             void applySlideToNoteAndVolumeSlideEffect(const mods::utils::RBuffer<Note>& note);
             void applyVibratoAndVolumeSlideEffect(const mods::utils::RBuffer<Note>& note);
             void applySetSampleOffsetEffect(const mods::utils::RBuffer<Note>& note);
             void applyVolumeSlideEffect(const mods::utils::RBuffer<Note>& note);
             void applyPositionJumpEffect(const mods::utils::RBuffer<Note>& note);
             void applySetVolumeEffect(const mods::utils::RBuffer<Note>& note);
             void applyPatternBreakEffect(const mods::utils::RBuffer<Note>& note);
             void applyExtendedEffect(const mods::utils::RBuffer<Note>& note);
             void applySetSpeedEffect(const mods::utils::RBuffer<Note>& note);
             
             void applyFineSlideUpEffect(const mods::utils::RBuffer<Note>& note);
             void applyPatternLoopEffect(const mods::utils::RBuffer<Note>& note);
             void applyRetriggerSampleEffect(const mods::utils::RBuffer<Note>& note);
             
             size_t _instrument = 0;
             size_t _currentSample = 0;
             size_t _currentRepeatSample = 0;
             u16 _period = 0;
             u16 _volume = 0;
             RLESample _currentValue;
             bool _speedSetOnLastLine = false;
             u32 _speed = 0;
             
             bool _hasPatternJump = false;
             int _patternOfJumpTarget = 0;
             int _lineOfJumpTarget = 0;
             
             bool _startOfLoop = false;
             bool _endOfLoop = false;
             u32 _loopLength = 0;
             
             const std::vector<mods::utils::RBuffer<s8>>* _sampleBuffers;
             mods::utils::RBuffer<Instrument> _instruments;
             
             constexpr static size_t NUMBER_OF_FINETUNE_FACTORS = 16;
             std::array<double, NUMBER_OF_FINETUNE_FACTORS> _fineTuneFactors;
             
             std::unique_ptr<NoEffect> _noEffect;
             std::unique_ptr<Vibrato> _vibrato;
             std::unique_ptr<VolumeSlide> _volumeSlide;
             std::unique_ptr<VibratoAndVolumeSlide> _vibratoAndVolumeSlide;
             std::unique_ptr<Arpeggio> _arpeggio;
             std::unique_ptr<SlideDown> _slideDown;
             std::unique_ptr<SlideUp> _slideUp;
             std::unique_ptr<SlideToNote> _slideToNote;
             std::unique_ptr<SlideToNoteAndVolumeSlide> _slideToNoteAndVolumeSlide;
             std::unique_ptr<Retrigger> _retrigger;
             Effect* _currentEffect = nullptr;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_CHANNELSTATE_HPP
