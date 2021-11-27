
#include "mods/ModuleReader.hpp"
#include "mods/mod/ModReader.hpp"
#include "mods/wav/WavReader.hpp"

namespace mods
{
   // static
   auto ModuleReader::parseFormat(const std::string& format) -> ModuleFormat
     {
        if(format == "wav")
          {
             return ModuleFormat::WAV;
          }
        if(format == "mod")
          {
             return ModuleFormat::MOD;
          }
        
        return ModuleFormat::UNKNOWN;
     }
   
   // static
   auto ModuleReader::buildReader(ModuleFormat format, const std::string& filename) -> ModuleReader::ptr
     {
        ModuleReader::ptr reader;
        switch(format) 
          {
           case ModuleFormat::WAV:
             reader = std::make_unique<wav::WavReader>(filename);
             break;
           case ModuleFormat::MOD:
             reader = std::make_unique<mod::ModReader>();
             break;
           case ModuleFormat::UNKNOWN:
             // keep null reader
             break;
          }
        return reader;
     }
   
   void ModuleReader::checkInit(bool condition, const std::string& description)
     {
        if(!condition)
          {
             throw ModuleReaderInitException(description);
          }
     }
   
   ModuleReader::ModuleReaderInitException::ModuleReaderInitException(const std::string& reason)
     : std::runtime_error(reason)
       {
       }
} // namespace mods
