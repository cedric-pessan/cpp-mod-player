
#include "mods/ModuleReader.hpp"
#include "mods/SoundPlayer.hpp"
#include "mods/utils/RBuffer.hpp"
#include "mods/utils/RBufferBackend.hpp"
#include "mods/utils/types.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace
{
   void printModuleInfo(const std::string& info)
     {
        std::cout << info << '\n';
     }
   
   void printModuleProgress(std::string progress)
     {
        static std::string lastLine;
        for(u32 i=progress.length(); i<lastLine.length(); ++i)
          {
             progress += ' ';
          }
        std::cout << '\r' << progress << std::flush;
        lastLine = progress;
     }
} // namespace

auto main(int argc, char** argv) -> int
{
   if(argc < 2)
     {
        std::cout << "no input file" << '\n';
        return 0;
     }
   
   auto deleter = std::make_unique<mods::utils::RBufferBackend::EmptyDeleter>();
   auto argBuffer = std::make_unique<mods::utils::RBufferBackend>(argv, argc, std::move(deleter));
   const mods::utils::RBuffer<char*> args(std::move(argBuffer));
   
   const std::string filename = args[1];
   auto dotIdx = filename.find_last_of('.');
   if(dotIdx == std::string::npos) 
     {
        std::cout << "count not find extension: " << filename << '\n';
        return 0;
     }
   std::string format = filename.substr(dotIdx+1);
   std::transform(format.begin(), format.end(), format.begin(), ::tolower);
   
   auto fmt = mods::ModuleReader::parseFormat(format);
   if(fmt == mods::ModuleFormat::UNKNOWN) 
     {
        std::cout << "unknown format:" << format << '\n';
        return 0;
     }
   
   auto reader = mods::ModuleReader::buildReader(fmt, filename);
   if(!reader)
     {
        std::cout << "could not initialize reader for " << filename << '\n';
        return 0;
     }
   
   mods::SoundPlayer player([](const std::string& info){ printModuleInfo(info); },
                            [](const std::string& progress){ printModuleProgress(progress); });
   player.play(std::move(reader));
   
   std::cout << '\n';
   
   return 0;
}
