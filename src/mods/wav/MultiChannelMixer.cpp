
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
                    for(size_t i = 0; i < _channels.size(); ++i)
                      {
                         _channelsBuffers.emplace_back(allocateNewTempBuffer());
                      }
                 }
             
             mods::utils::RWBuffer<u8> InternalMultiChannelMixerSourceConverter::allocateNewTempBuffer()
               {
                  std::cout << "InternalMultiChannelMixerSourceConverter::allocateNewTempBuffer()" << std::endl;
               }
             
             void InternalMultiChannelMixerSourceConverter::ensureChannelBuffersSizes()
               {
                  std::cout << "TODO: InternalMultiChannelMixerSourceConverter::ensureChannelBuffersSizes()" << std::endl;
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
             
             void InternalMultiChannelMixerSourceConverter::read(mods::utils::RWBuffer<u8>* buf, int len, ChannelId outChannel)
               {
                  if((len % sizeof(double)) != 0)
                    {
                       std::cout << "TODO: wrong buffer length in InternalMultiChannelMixerSourceConverter" << std::endl;
                    }
                  
                  size_t toRead = len / sizeof(double);
                  int read = 0;
                  auto idxBuffer = toUnderlying(outChannel);
                  auto outView = buf->slice<double>(0, toRead);
                  
                  while(!_unconsumedBuffers.at(idxBuffer).empty() && read < len)
                    std::cout << "TODO: InternalMultiChannelMixerSourceCoverter::read(mods::utils::RWBuffer<u8>*, int, ChannelId) unconsumed loop" << std::endl;
                  if(read < len) 
                    {
                       auto remainsToRead = toRead - read;
                       ensureChannelBuffersSizes();
                       for(size_t i=0; i<_channels.size(); ++i)
                         {
                            auto& channel = _channels[i];
                            auto& tempChannelBuffer = _channelsBuffers[i];
                            channel->read(&tempChannelBuffer, remainsToRead);
                         }
                       
                       for(size_t i=0; i<remainsToRead; ++i)
                         {
                            double sample = mix();
                            outView[read++] = sample;
                            
                            sample = mix();
                            _unconsumedBuffers.at(1-idxBuffer).push_back(sample);
                         }
                    }
               }
             
             double InternalMultiChannelMixerSourceConverter::mix() const
               {
                  std::cout << "TODO: InternalMultiChannelMixerSourceConverter::mix() const" << std::endl;
                  return 0.0;
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
                  _src->read(buf, len, _channel);
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