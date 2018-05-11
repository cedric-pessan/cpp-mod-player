
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
        
        virtual bool isValid() const = 0;
        
      protected:
        ModuleReader();
        
      private:
        ModuleReader(const ModuleReader&);
        ModuleReader& operator=(const ModuleReader&);
     };
   
}

#endif // _MODULEREADER_HPP_
