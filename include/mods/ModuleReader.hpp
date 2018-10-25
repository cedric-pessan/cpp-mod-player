
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
        UNKNOWN
     };
   
   class ModuleReader
     {
      public:
        using ptr = std::unique_ptr<ModuleReader>;
        
        ModuleReader(const ModuleReader&) = delete;
        ModuleReader(const ModuleReader&&) = delete;
        ModuleReader& operator=(const ModuleReader&) = delete;
        ModuleReader& operator=(const ModuleReader&&) = delete;
        virtual ~ModuleReader() = default;
        
        virtual bool isFinished() const = 0;
        virtual void read(mods::utils::RWBuffer<u8>* buf, int len) = 0;
        virtual std::string getInfo() const = 0;
        virtual std::string getProgressInfo() const = 0;
        
        static ModuleFormat parseFormat(const std::string& format);
        static ModuleReader::ptr buildReader(ModuleFormat format, const std::string& filename);
        
      protected:
        ModuleReader() = default;
        
        void checkInit(bool condition, const std::string& description) const;
        
      private:
        
        class ModuleReaderInitException : public std::runtime_error
          {
           public:
             explicit ModuleReaderInitException(const std::string& reason);
             ModuleReaderInitException(const ModuleReaderInitException&) noexcept = default;
             ModuleReaderInitException(ModuleReaderInitException&&) noexcept = default;
             ~ModuleReaderInitException() override = default;
             
             ModuleReaderInitException() = delete;
             ModuleReaderInitException& operator=(const ModuleReaderInitException&) = delete;
             ModuleReaderInitException& operator=(const ModuleReaderInitException&&) = delete;
          };
     };
   
} // namespace mods

#endif // MODS_MODULEREADER_HPP
