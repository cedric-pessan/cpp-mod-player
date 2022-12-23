#ifndef MODS_MOD_MODREADER_HPP
#define MODS_MOD_MODREADER_HPP

#include "mods/ModuleReader.hpp"
#include "mods/mod/Instrument.hpp"
#include "mods/mod/PatternReader.hpp"

namespace mods
{
   namespace mod
     {
        class ModReader : public ModuleReader
          {
           public:
             ModReader(const std::string& fileName);
             ~ModReader() override = default;
             
             ModReader() = delete;
             ModReader(const ModReader&) = delete;
             ModReader(ModReader&&) = delete;
             auto operator=(const ModReader&) -> ModReader& = delete;
             auto operator=(ModReader&&) -> ModReader& = delete;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<s16>* buf) override;
             auto getInfo() const -> std::string override;
             auto getProgressInfo() const -> std::string override;
             
           private:
             auto parseSongTitle() -> std::string;
             auto parseInstruments() -> mods::utils::RBuffer<Instrument>;
             auto detectNumberOfInstruments() const -> u32;
             auto parseNumberOfPatterns() -> size_t;
             auto parseEndJumpPosition() -> size_t;
             auto parsePatternsTable() -> mods::utils::RBuffer<u8>;
             auto getNumberOfChannelsFromFormatTag() -> size_t;
             auto parsePatternsBuffer() -> mods::utils::RBuffer<Note>;
             auto parseSampleBuffers() -> std::vector<mods::utils::RBuffer<u8>>;
             auto getPatternBuffer(size_t patternIndex) -> mods::utils::RBuffer<Note>;
             
             auto getSongTitle() const -> std::string;
             
             constexpr static int _songFieldLength = 20;
             
             const mods::utils::RBuffer<u8> _fileBuffer;
             mods::utils::RBuffer<u8> _notParsedBuffer;
             std::string _songTitle;
             u32 _numberOfInstruments;
             mods::utils::RBuffer<Instrument> _instruments;
             size_t _numberOfPatterns;
             size_t _endJumpPosition;
             
             mods::utils::RBuffer<u8> _patternsOrderList;
             
             size_t _nbChannels;
             
             mods::utils::RBuffer<Note> _patterns;
             std::vector<mods::utils::RBuffer<u8>> _sampleBuffers;
             
             PatternReader _patternReader;
             size_t _currentPatternIndex = 0;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_MODREADER_HPP
