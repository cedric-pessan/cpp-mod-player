
#include "mods/wav/MultiChannelMixer.hpp"

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             InternalMultiChannelMixerSourceConverter::InternalMultiChannelMixerSourceConverter(std::vector<WavConverter::ptr> channels)
               : _channels(std::move(channels))
                 {
                 }
             
             bool InternalMultiChannelMixerSourceConverter::isFinished(ChannelId outChannel) const
               {
                  auto idxBuffer = toUnderlying(outChannel);
                  if(!_unconsumedBuffers.at(idxBuffer).empty())
                    {
                       return false;
                    }
                  for(auto& channel : _channels) 
                    {
                       if(!channel->isFinished())
                         {
                            return false;
                         }
                    }
                  return true;
               }
             
             MultiChannelMixerBase::MultiChannelMixerBase(InternalMultiChannelMixerSourceConverter::sptr src, ChannelId channel)
               : _src(std::move(src)),
               _channel(channel)
                 {
                 }
             
             WavConverter::ptr MultiChannelMixerBase::buildRightChannel() const
               {
                  class make_unique_enabler : public MultiChannelMixerBase
                    {
                     public:
                       explicit make_unique_enabler(const InternalMultiChannelMixerSourceConverter::sptr& src)
                         : MultiChannelMixerBase(src, ChannelId::RIGHT)
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
             
             bool MultiChannelMixerBase::isFinished() const
               {
                  return _src->isFinished(_channel);
               }
             
             void MultiChannelMixerBase::read(mods::utils::RWBuffer<u8>* buf, int len)
               {
                  std::cout << "TODO: MultiChannelMixerSlave::read(...)" << std::endl;
               }
          } // namespace impl
        
        MultiChannelMixer::MultiChannelMixer(std::vector<WavConverter::ptr> channels)
          : MultiChannelMixerBase(std::make_shared<impl::InternalMultiChannelMixerSourceConverter>(std::move(channels)), impl::ChannelId::LEFT),
          _right(buildRightChannel())
            {
            }
        
        WavConverter::ptr MultiChannelMixer::getRightChannel()
          {
             return std::move(_right);
          }
     } // namespace wav
} // namespace mods