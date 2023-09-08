#ifndef MODS_MOD_MODREADER_HPP
#define MODS_MOD_MODREADER_HPP

#include "mods/ModuleReader.hpp"
#include "mods/converters/Converter.hpp"
#include "mods/mod/PatternListReader.hpp"
#include "mods/utils/AmigaRLESample.hpp"

namespace mods
{
   namespace mod
     {
        class Instrument;
        class PatternListReader;
        
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
             
             auto getSongTitle() const -> std::string;
             
             template<typename T>
               using Converter = mods::converters::Converter<T>;
             using RLESample = mods::utils::AmigaRLESample;
             
             static auto buildModConverter(std::shared_ptr<PatternListReader> patternListReader) -> Converter<s16>::ptr;
             
             static auto buildResamplerStage(Converter<RLESample>::ptr left,
                                             Converter<RLESample>::ptr right) -> Converter<s16>::ptr;
             
             static auto buildResampler(Converter<RLESample>::ptr channel) -> Converter<double>::ptr;
             
             static auto buildFromDoubleStage(Converter<double>::ptr left,
                                              Converter<double>::ptr right) -> Converter<s16>::ptr;
             
             static auto buildMuxStage(Converter<s16>::ptr left,
                                       Converter<s16>::ptr right) -> Converter<s16>::ptr;
             
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
             
             std::shared_ptr<PatternListReader> _patternListReader;
             mods::converters::Converter<s16>::ptr _modConverter;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_MODREADER_HPP
