
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
        virtual ~ModuleReader();
        
        static ModuleFormat parseFormat(const std::string& format);
        static std::unique_ptr<ModuleReader> buildReader(ModuleFormat format);
        
        void waitUntilFinished();
        
      protected:
        ModuleReader();
        
      private:
        ModuleReader(const ModuleReader&);
        ModuleReader& operator=(const ModuleReader&);
        
        class ModuleReaderInitException : public std::exception
          {
           public:
             ModuleReaderInitException(const ModuleReaderInitException&);
             virtual ~ModuleReaderInitException();
             virtual const char* what() const noexcept override;
             
           private:
             ModuleReaderInitException();
             ModuleReaderInitException& operator=(const ModuleReaderInitException&);
             
             const std::string _reason;
          };
     };
   
}

#endif // _MODULEREADER_HPP_
