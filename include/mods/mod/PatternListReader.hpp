#ifndef MODS_MOD_PATTERNLISTREADER_HPP
#define MODS_MOD_PATTERNLISTREADER_HPP

#include "mods/mod/ChannelId.hpp"
#include "mods/mod/PatternReader.hpp"
#include "mods/utils/AmigaRLESample.hpp"
#include "mods/utils/DynamicRingBuffer.hpp"
#include "mods/utils/RWBuffer.hpp"

#include <array>

namespace mods
{
   namespace mod
     {
        class PatternListReader
          {
           private:
             using RLESample = mods::utils::AmigaRLESample;
             
           public:
             PatternListReader(size_t numberOfPatterns,
                               const mods::utils::RBuffer<u8>& patternsOrderList,
                               size_t nbChannels,
                               const mods::utils::RBuffer<Note>& patterns,
                               const mods::utils::RBuffer<Instrument>& instruments,
                               const std::vector<mods::utils::RBuffer<u8>>& sampleBuffers);
             
             PatternListReader() = delete;
             PatternListReader(const PatternListReader&) = delete;
             PatternListReader(PatternListReader&&) = delete;
             auto operator=(const PatternListReader&) -> PatternListReader& = delete;
             auto operator=(PatternListReader&&) -> PatternListReader& = delete;
             ~PatternListReader() = default;
             
             auto isChannelFinished(ChannelId channel) const -> bool;
             void readChannel(mods::utils::RWBuffer<RLESample>* buf, ChannelId channel);
             
             auto getProgressInfo() const -> std::string;
             
           private:
             auto getPatternBuffer(size_t patternIndex) -> mods::utils::RBuffer<Note>;
             
             using UnconsumedBuffer = mods::utils::DynamicRingBuffer<RLESample>;
             std::array<UnconsumedBuffer,2> _unconsumedBuffers;
             
             size_t _currentPatternIndex = 0;
             size_t _numberOfPatterns;
             
             mods::utils::RBuffer<u8> _patternsOrderList;
             
             size_t _nbChannels;
             
             mods::utils::RBuffer<Note> _patterns;
             
             PatternReader _patternReader;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_PATTERNLISTREADER_HPP
