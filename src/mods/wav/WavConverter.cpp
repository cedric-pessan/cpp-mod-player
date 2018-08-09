
#include "mods/wav/WavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        // static
        WavConverter::ptr WavConverter::buildConverter(int bitsPerSample)
          {
             std::cout << "TODO: WavConverter:: bitsPerSample: " << bitsPerSample << std::endl;
             
             std::cout << "TODO: WavConverter::buildConverter()" << std::endl;
             return nullptr;
          }
     }
} // namespace mods
