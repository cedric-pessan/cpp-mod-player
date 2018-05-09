
#ifndef _MODREADER_HPP_
#define _MODREADER_HPP_

#include <string>
#include <memory>

namespace mods
{
   enum struct ModFormat
     {
        Unknown
     };
   
   class ModReader
     {
      public:
        ~ModReader();
        
        static ModFormat parseFormat(const std::string& format);
        static std::unique_ptr<ModReader> buildReader();
        
      private:
        ModReader();
        ModReader(const ModReader&);
        ModReader& operator=(const ModReader&);
     };
   
}

#endif // _MODREADER_HPP_
