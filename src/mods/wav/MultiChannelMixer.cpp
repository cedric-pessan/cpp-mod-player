
#include "mods/wav/MultiChannelMixer.hpp"

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             WavConverter::ptr MultiChannelMixerSlave::buildSlave() const
               {
                  return std::make_unique<MultiChannelMixerSlave>();
               }
             
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
        
        MultiChannelMixer::MultiChannelMixer()
          : _right(buildSlave())
            {
            }
        
        WavConverter::ptr MultiChannelMixer::getRightChannel()
          {
             return std::move(_right);
          }
     } // namespace wav
} // namespace mods