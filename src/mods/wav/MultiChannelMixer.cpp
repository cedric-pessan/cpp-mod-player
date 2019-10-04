
#include "mods/wav/MultiChannelMixer.hpp"

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             bool InternalMultiChannelMixerSourceConverter::isFinished() const
               {
                  std::cout << "TODO: InternalMultiChannelMixerSourceConverter::isFinished() const" << std::endl;
                  return false;
               }
             
             MultiChannelMixerSlave::MultiChannelMixerSlave(InternalMultiChannelMixerSourceConverter::sptr src)
               : _src(std::move(src))
                 {
                 }
             
             WavConverter::ptr MultiChannelMixerSlave::buildSlave() const
               {
                  class make_unique_enabler : public MultiChannelMixerSlave
                    {
                     public:
                       explicit make_unique_enabler(const InternalMultiChannelMixerSourceConverter::sptr& src)
                         : MultiChannelMixerSlave(src)
                           {
                           }
                       
                       make_unique_enabler() = delete;
                       make_unique_enabler(const make_unique_enabler&) = delete;
                       make_unique_enabler(make_unique_enabler&&) = delete;
                       make_unique_enabler& operator=(const make_unique_enabler&) = delete;
                       make_unique_enabler& operator=(make_unique_enabler&&) = delete;
                       ~make_unique_enabler() override = default;
                    };
                  return std::make_unique<make_unique_enabler>(_src);
               }
             
             bool MultiChannelMixerSlave::isFinished() const
               {
                  return _src->isFinished();
               }
             
             void MultiChannelMixerSlave::read(mods::utils::RWBuffer<u8>* buf, int len)
               {
                  std::cout << "TODO: MultiChannelMixerSlave::read(...)" << std::endl;
               }
          } // namespace impl
        
        MultiChannelMixer::MultiChannelMixer(/*std::vector<WavConverter::ptr> channels*/)
          : MultiChannelMixerSlave(std::make_shared<impl::InternalMultiChannelMixerSourceConverter>(/*std::move(channels)*/)),
          _right(buildSlave())
            {
            }
        
        WavConverter::ptr MultiChannelMixer::getRightChannel()
          {
             return std::move(_right);
          }
     } // namespace wav
} // namespace mods