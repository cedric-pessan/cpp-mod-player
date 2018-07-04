
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
        
        virtual ~ModuleReader();
        
        virtual bool isFinished() const = 0;
        
        static ModuleFormat parseFormat(const std::string& format);
        static ModuleReader::ptr buildReader(ModuleFormat format, const std::string& filename);
        
      protected:
        ModuleReader();
        
        void checkInit(bool condition, const std::string& description);
        
      private:
        ModuleReader(const ModuleReader&) = delete;
        ModuleReader& operator=(const ModuleReader&) = delete;
        
        class ModuleReaderInitException : public std::exception
          {
           public:
             explicit ModuleReaderInitException(const std::string& reason);
             ModuleReaderInitException(const ModuleReaderInitException&);
             virtual ~ModuleReaderInitException();
             virtual const char* what() const noexcept override;
             
           private:
             ModuleReaderInitException() = delete;
             ModuleReaderInitException& operator=(const ModuleReaderInitException&) = delete;
             
             const std::string _reason;
          };
     };
   
}

#endif // _MODULEREADER_HPP_
