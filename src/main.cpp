
#include "mods/ModuleReader.hpp"
#include "mods/utils/RBuffer.hpp"
#include "mods/SoundPlayer.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

namespace
{
   class EmptyDeleter : public mods::utils::Buffer::Deleter
     {
      public:
        EmptyDeleter() = default;
        ~EmptyDeleter() override = default;
        
        EmptyDeleter(const EmptyDeleter&) = delete;
        EmptyDeleter(const EmptyDeleter&&) = delete;
        EmptyDeleter& operator=(const EmptyDeleter&) = delete;
        EmptyDeleter& operator=(const EmptyDeleter&&) = delete;
     };
} // namespace

int main(int argc, char** argv)
{
   if(argc < 2)
     {
        std::cout << "no input file" << std::endl;
        return 0;
     }
   
   auto deleter = std::make_unique<EmptyDeleter>();
   auto argvp = static_cast<u8*>(static_cast<void*>(argv));
   auto argBuffer = std::make_shared<mods::utils::Buffer>(argvp, argc * sizeof(char*), std::move(deleter));
   mods::utils::RBuffer<char*> args(argBuffer);
   
   std::string filename = args[1];
   auto dotIdx = filename.find_last_of('.');
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
   
   auto reader = mods::ModuleReader::buildReader(fmt, filename);
   if(!reader)
     {
        std::cout << "could not initialize reader for " << filename << std::endl;
        return 0;
     }
   
   mods::SoundPlayer player;
   player.play(std::move(reader));
   
   return 0;
}
