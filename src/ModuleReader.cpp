
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
        try
          {
             switch(format) 
               {
                case ModuleFormat::WAV:
                  reader = std::make_unique<WavReader>();
                  break;
                case ModuleFormat::UNKNOWN:
                  // keep null reader
                  break;
               }
          }
        catch(ModuleReaderInitException ex)
          {
          }
        return reader;
     }
   
   ModuleReader::ModuleReader()
     {
     }
   
   ModuleReader::~ModuleReader()
     {
     }
   
   void ModuleReader::waitUntilFinished()
     {
        std::cout << "TODO: ModuleReader::waitUnitFinished()" << std::endl;
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
