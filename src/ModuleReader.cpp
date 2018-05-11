
#include "ModuleReader.hpp"
#include "WavReader.hpp"

#include <iostream>

namespace mods
{
   // static
   ModuleFormat ModuleReader::parseFormat(const std::string& format)
     {
        if(format == "wav") 
          return ModuleFormat::WAV;
        
        return ModuleFormat::UNKNOWN;
     }
   
   // static
   std::unique_ptr<ModuleReader> ModuleReader::buildReader(ModuleFormat format)
     {
        std::unique_ptr<ModuleReader> reader;
        switch(format) 
          {
           case ModuleFormat::WAV:
             reader = std::make_unique<WavReader>();
             break;
           case ModuleFormat::UNKNOWN:
             // keep null reader
             break;
          }
        if(reader && !reader->isValid()) 
          {
             return std::unique_ptr<ModuleReader>(nullptr);
          }
        return reader;
     }
   
   ModuleReader::ModuleReader()
     : _valid(true)
     {
     }
   
   ModuleReader::~ModuleReader()
     {
     }
   
   bool ModuleReader::isValid() const
     {
        return _valid;
     }
   
}
