
#ifndef MODS_MODULEREADER_HPP
#define MODS_MODULEREADER_HPP

#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"

#include <memory>
#include <string>

namespace mods
{
   enum struct ModuleFormat
     {
        WAV,
          MOD,
          UNKNOWN
     };
   
   class ModuleReader
     {
      public:
        using ptr = std::unique_ptr<ModuleReader>;
        
        ModuleReader(const ModuleReader&) = delete;
        ModuleReader(ModuleReader&&) = delete;
        auto operator=(const ModuleReader&) -> ModuleReader& = delete;
        auto operator=(ModuleReader&&) -> ModuleReader& = delete;
        virtual ~ModuleReader() = default;
        
        virtual auto isFinished() const -> bool = 0;
        virtual void read(mods::utils::RWBuffer<s16>* buf) = 0;
        virtual auto getInfo() const -> std::string = 0;
        virtual auto getProgressInfo() const -> std::string = 0;
        
        static auto parseFormat(const std::string& format) -> ModuleFormat;
        static auto buildReader(ModuleFormat format, const std::string& filename) -> ModuleReader::ptr;
        
      protected:
        ModuleReader() = default;
        
        static void checkInit(bool condition, const std::string& description);
        
      private:
        
        class ModuleReaderInitException : public std::runtime_error
          {
           public:
             explicit ModuleReaderInitException(const std::string& reason);
             ModuleReaderInitException(const ModuleReaderInitException&) noexcept = delete;
             ModuleReaderInitException(ModuleReaderInitException&&) noexcept = default;
             ~ModuleReaderInitException() override = default;
             
             ModuleReaderInitException() = delete;
             auto operator=(const ModuleReaderInitException&) -> ModuleReaderInitException& = delete;
             auto operator=(ModuleReaderInitException&&) -> ModuleReaderInitException& = delete;
          };
     };
   
} // namespace mods

#endif // MODS_MODULEREADER_HPP
