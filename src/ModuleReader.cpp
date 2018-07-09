
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
   
   ModuleReader::ModuleReader()
     {
     }
   
   ModuleReader::~ModuleReader()
     {
     }
   
   void ModuleReader::checkInit(bool condition, const std::string& description)
     {
        if(!condition) throw ModuleReaderInitException(description);
     }
   
   ModuleReader::ModuleReaderInitException::ModuleReaderInitException(const std::string& reason)
     : _reason(reason)
       {
       }
   
   ModuleReader::ModuleReaderInitException::ModuleReaderInitException(const ModuleReaderInitException& ex)
     : std::exception(ex),
     _reason(ex._reason)
     {
     }
   
   ModuleReader::ModuleReaderInitException::~ModuleReaderInitException()
     {
     }
   
   const char* ModuleReader::ModuleReaderInitException::what() const noexcept
     {
        return _reason.c_str();
     }
}
