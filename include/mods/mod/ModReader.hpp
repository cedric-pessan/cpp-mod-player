#ifndef MODS_MOD_MODREADER_HPP
#define MODS_MOD_MODREADER_HPP

#include "mods/ModuleReader.hpp"

namespace mods
{
   namespace mod
     {
        class ModReader : public ModuleReader
          {
           public:
             ModReader() = default;
             ~ModReader() override = default;
             
             ModReader(const ModReader&) = delete;
             ModReader(ModReader&&) = delete;
             auto operator=(const ModReader&) -> ModReader& = delete;
             auto operator=(ModReader&&) -> ModReader& = delete;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             auto getInfo() const -> std::string override;
             auto getProgressInfo() const -> std::string override;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_MODREADER_HPP
