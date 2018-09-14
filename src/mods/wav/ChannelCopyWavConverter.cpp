
#include "mods/wav/ChannelCopyWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        bool ChannelCopyWavConverter::isFinished() const
          {
             std::cout << "TODO: ChannelCopyWavConverter::isFinished() const" << std::endl;
             return false;
          }
        
        WavConverter::ptr ChannelCopyWavConverter::getCopy()
          {
             std::cout << "TODO: ChannelCopyWavConverter::getCopy()" << std::endl;
             return nullptr;
          }
     } // namespace wav
} // namespace mods
