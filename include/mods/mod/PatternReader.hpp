#ifndef MODS_MOD_PATTERNREADER_HPP
#define MODS_MOD_PATTERNREADER_HPP

#include "mods/mod/ChannelState.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"

#include <vector>

namespace mods
{
   namespace mod
     {
        class PatternReader
          {
           public:
             PatternReader(size_t nbChannels,
                           const mods::utils::RBuffer<Note>& patternBuffer,
                           const std::vector<mods::utils::RBuffer<u8>>& sampleBuffers);
             
             PatternReader() = delete;
             PatternReader(const PatternReader&) = delete;
             PatternReader(PatternReader&&) = delete;
             auto operator=(const PatternReader&) -> PatternReader& = delete;
             auto operator=(PatternReader&&) -> PatternReader& = delete;
             ~PatternReader() = default;
             
             auto isFinished() const -> bool;
             auto isTickFinished() const -> bool;
             auto readTickBuffer(size_t nbElems) -> mods::utils::RBuffer<s16>;
             void readNextTick();
             void setPattern();
             
             auto constexpr static getNumberOfLines() -> size_t
               {
                  return _numberOfLines;
               }
             
           private:
             auto computeTickBufferLength() const -> size_t;
             auto allocateTickBuffer(size_t len) -> mods::utils::RWBuffer<s16>;
             
             auto isLineFinished() const -> bool;
             
             auto readAndMixSample(const std::vector<ChannelState*>& channels) const -> s16;
             
             void decodeLine();
             
             static constexpr u32 _numberOfLines = 64;
             static constexpr u32 _defaultSpeed = 6;
             static constexpr u32 _defaultBpm = 125;
             
             u32 _speed = _defaultSpeed;
             u32 _bpm = _defaultBpm;
             
             u32 _currentLine = 0;
             u32 _currentTick = 0;
             
             std::vector<u8> _tickVec;
             mods::utils::RWBuffer<s16> _tickBuffer;
             mods::utils::RBuffer<s16> _unreadTickBuffer;
             
             std::vector<ChannelState> _channels;
             std::vector<ChannelState*> _leftChannels;
             std::vector<ChannelState*> _rightChannels;
             
             mods::utils::RBuffer<Note> _patternBuffer;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_PATTERNREADER_HPP
