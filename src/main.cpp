
#include "ModuleReader.hpp"
#include "SoundPlayer.hpp"

#include <iostream>
#include <algorithm>
#include <memory>

int main(int argc, char** argv)
{
   if(argc < 2)
     {
        std::cout << "no input file" << std::endl;
        return 0;
     }
   
   std::string filename = argv[1];
   auto dotIdx = filename.find_last_of(".");
   if(dotIdx == std::string::npos) 
     {
        std::cout << "count not find extension: " << filename << std::endl;
        return 0;
     }
   std::string format = filename.substr(dotIdx+1);
   std::transform(format.begin(), format.end(), format.begin(), ::tolower);
   
   auto fmt = mods::ModuleReader::parseFormat(format);
   if(fmt == mods::ModuleFormat::UNKNOWN) 
     {
        std::cout << "unknown format:" << format << std::endl;
        return 0;
     }
   
   auto reader = mods::ModuleReader::buildReader(fmt);
   if(!reader)
     {
        std::cout << "could not initialize reader for " << filename << std::endl;
        return 0;
     }
   
   mods::SoundPlayer player;
   player.play(std::move(reader));
   
   return 0;
}
