
#ifndef _MODULEREADER_HPP_
#define _MODULEREADER_HPP_

#include <string>
#include <memory>

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
        typedef std::unique_ptr<ModuleReader> ptr;
        
        virtual ~ModuleReader() = default;
        
        virtual bool isFinished() const = 0;
        
        static ModuleFormat parseFormat(const std::string& format);
        static ModuleReader::ptr buildReader(ModuleFormat format, const std::string& filename);
        
      protected:
        ModuleReader() = default;
        
        void checkInit(bool condition, const std::string& description) const;
        
      private:
        ModuleReader(const ModuleReader&) = delete;
        ModuleReader& operator=(const ModuleReader&) = delete;
        
        class ModuleReaderInitException : public std::runtime_error
          {
           public:
             explicit ModuleReaderInitException(const std::string& reason);
             ModuleReaderInitException(const ModuleReaderInitException&) noexcept = default;
             virtual ~ModuleReaderInitException() = default;
             
           private:
             ModuleReaderInitException() = delete;
             ModuleReaderInitException& operator=(const ModuleReaderInitException&) = delete;
          };
     };
   
}

#endif // _MODULEREADER_HPP_
