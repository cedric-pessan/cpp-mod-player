
#include "ModuleReader.hpp"
#include "WavReader.hpp"

#include <iostream>

namespace mods
{
   // static
   ModuleFormat ModuleReader::parseFormat(const std::string& format)
     {
        if(format == "wav")
          {
             return ModuleFormat::WAV;
          }
        
        return ModuleFormat::UNKNOWN;
     }
   
   // static
   ModuleReader::ptr ModuleReader::buildReader(ModuleFormat format, const std::string& filename)
     {
        ModuleReader::ptr reader;
        switch(format) 
          {
           case ModuleFormat::WAV:
             reader = std::make_unique<WavReader>(filename);
             break;
           case ModuleFormat::UNKNOWN:
             // keep null reader
             break;
          }
        return reader;
     }
   
   void ModuleReader::checkInit(bool condition, const std::string& description) const
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
