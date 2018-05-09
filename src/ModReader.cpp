
#include "ModReader.hpp"

#include <iostream>

namespace mods
{
   // static
   ModFormat ModReader::parseFormat(const std::string& format)
     {
        std::cout << "TODO: ModReader::parseFormat" << std::endl;
        return ModFormat::Unknown;
     }
   
   // static
   std::unique_ptr<ModReader> ModReader::buildReader()
     {
        std::cout << "TODO: ModReader::buildReader" << std::endl;
        return std::unique_ptr<ModReader>(nullptr);
     }
   
   ModReader::~ModReader()
     {
        std::cout << "TODO: ModReader::~ModReader()" << std::endl;
     }
   
}
