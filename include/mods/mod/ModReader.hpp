#ifndef MODS_MOD_MODREADER_HPP
#define MODS_MOD_MODREADER_HPP

#include "mods/ModuleReader.hpp"
#include "mods/mod/Instrument.hpp"

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
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             auto getInfo() const -> std::string override;
             auto getProgressInfo() const -> std::string override;
             
           private:
             auto initializeSongTitle() -> std::string;
             auto initializeInstruments() -> mods::utils::RBuffer<Instrument>;
             auto detectNumberOfInstruments() const -> u32;
             
             auto getSongTitle() const -> std::string;
             
             constexpr static int _songFieldLength = 20;
             
             const mods::utils::RBuffer<u8> _fileBuffer;
             mods::utils::RBuffer<u8> _notParsedBuffer;
             std::string _songTitle;
             u32 _numberOfInstruments;
             mods::utils::RBuffer<Instrument> _instruments;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_MODREADER_HPP
