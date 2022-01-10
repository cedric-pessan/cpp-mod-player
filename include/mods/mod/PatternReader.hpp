#ifndef MODS_MOD_PATTERNREADER_HPP
#define MODS_MOD_PATTERNREADER_HPP

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
             PatternReader() /*= default*/;
             PatternReader(const PatternReader&) = delete;
             PatternReader(PatternReader&&) = delete;
             auto operator=(const PatternReader&) -> PatternReader& = delete;
             auto operator=(PatternReader&&) -> PatternReader& = delete;
             ~PatternReader() = default;
             
             auto isFinished() const -> bool;
             auto isTickFinished() const -> bool;
             auto readTickBuffer(size_t nbElems) const -> mods::utils::RBuffer<s16>;
             void readNextTick();
             void setPattern();
             
           private:
             auto computeTickBufferLength() const -> size_t;
             auto allocateTickBuffer(size_t len) -> mods::utils::RWBuffer<s16>;
             
             auto isLineFinished() const -> bool;
             
             static constexpr u32 _numberOfLines = 64;
             static constexpr u32 _defaultSpeed = 6;
             
             u32 _speed = _defaultSpeed;
             
             u32 _currentLine = 0;
             u32 _currentTick = 0;
             
             std::vector<u8> _tickVec;
             mods::utils::RWBuffer<s16> _tickBuffer;
             mods::utils::RBuffer<s16> _unreadTickBuffer;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_PATTERNREADER_HPP
