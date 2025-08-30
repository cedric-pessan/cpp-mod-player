#ifndef MODS_MOD_PATTERNREADER_HPP
#define MODS_MOD_PATTERNREADER_HPP

#include "mods/mod/ChannelState.hpp"
#include "mods/mod/Note.hpp"
#include "mods/utils/AmigaRLESample.hpp"
#include "mods/utils/DynamicRingBuffer.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"

#include <vector>

namespace mods
{
   namespace mod
     {
        class PatternReader
          {
           private:
             using RLESample = mods::utils::AmigaRLESample;
             using OutputQueue = mods::utils::DynamicRingBuffer<RLESample>;
             
             
           public:
             PatternReader(size_t nbChannels,
                           const mods::utils::RBuffer<Note>& patternBuffer,
                           const mods::utils::RBuffer<Instrument>& instruments,
                           const std::vector<mods::utils::RBuffer<s8>>& sampleBuffers,
                           std::array<OutputQueue, 2>* outputs);
             
             PatternReader() = delete;
             PatternReader(const PatternReader&) = delete;
             PatternReader(PatternReader&&) = delete;
             auto operator=(const PatternReader&) -> PatternReader& = delete;
             auto operator=(PatternReader&&) -> PatternReader& = delete;
             ~PatternReader() = default;
             
             auto isFinished() const -> bool;
             void readNextTick();
             void setPattern(const mods::utils::RBuffer<Note>& patternBuffer, int initialLine);
             
             auto getCurrentLine() const -> size_t;
             
             auto constexpr static getNumberOfLines() -> size_t
               {
                  return _numberOfLines;
               }
             
             auto hasPatternJump() const -> bool;
             auto getPatternOfJumpTarget() const -> int;
             auto getLineOfJumpTarget() const -> int;
             
           private:
             auto computeTickLength() const -> size_t;
             
             void readAndMixTick(OutputQueue* output, const std::vector<ChannelState*>& channels);
             static auto readAndMixSample(const std::vector<ChannelState*>& channels, u32 maxLength) -> RLESample;
             
             void decodeLine();
             void updateSpeed();
             void updatePatternJump();
             void updateLoop();
             void signalNewTick();
             void updateFilter();
             
             static constexpr u32 _numberOfLines = 64;
             static constexpr u32 _defaultSpeed = 6;
             static constexpr u32 _defaultBpm = 125;
             
             u32 _speed = _defaultSpeed;
             u32 _bpm = _defaultBpm;
             
             u32 _currentLine = 0;
             u32 _currentTick = 0;
             
             bool _patternJump = false;
             int _patternOfJumpTarget = 0;
             int _lineOfJumpTarget = 0;
             
             u32 _startOfLoop = 0;
             u32 _remainingLoop = 0;
             
             std::vector<ChannelState> _channels;
             std::vector<ChannelState*> _leftChannels;
             std::vector<ChannelState*> _rightChannels;
             
             mods::utils::RBuffer<Note> _patternBuffer;
             
             OutputQueue* _leftOutput;
             OutputQueue* _rightOutput;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_PATTERNREADER_HPP
