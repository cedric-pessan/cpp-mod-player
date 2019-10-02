
#include "mods/wav/MultiChannelMixer.hpp"

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             bool MultiChannelMixerSlave::isFinished() const
               {
                  std::cout << "TODO: MultiChannelMixerSlave::isFinished() const" << std::endl;
                  return false;
               }
             
             void MultiChannelMixerSlave::read(mods::utils::RWBuffer<u8>* buf, int len)
               {
                  std::cout << "TODO: MultiChannelMixerSlave::read(...)" << std::endl;
               }
          } // namespace impl
        
        WavConverter::ptr MultiChannelMixer::getRight()
          {
             std::cout << "TODO: MultiChannelMixer::getRight()" << std::endl;
             return nullptr;
          }
     } // namespace wav
} // namespace mods