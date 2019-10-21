
#include "mods/wav/MultiChannelMixer.hpp"

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             InternalMultiChannelMixerSourceConverter::InternalMultiChannelMixerSourceConverter(std::vector<WavConverter::ptr> channels, u32 channelMask)
               : _channels(std::move(channels)),
               _channelMask(channelMask)
                 {
                    for(size_t i = 0; i < _channels.size(); ++i)
                      {
                         _channelsVec.emplace_back();
                         _channelsBuffers.emplace_back(allocateNewTempBuffer(&_channelsVec.back(), 0));
                         _channelsViews.emplace_back(_channelsBuffers.back().slice<double>(0, 0));
                      }
                    
                    computeMixingCoefficients();
                 }
             
             mods::utils::RWBuffer<u8> InternalMultiChannelMixerSourceConverter::allocateNewTempBuffer(std::vector<u8>* backendVec, size_t len)
               {
                  backendVec->resize(len);
                  u8* ptr = backendVec->data();
                  auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
                  auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
                  return mods::utils::RWBuffer<u8>(buffer);
               }
             
             void InternalMultiChannelMixerSourceConverter::ensureChannelBuffersSizes(size_t len)
               {
                  for(size_t i=0; i<_channels.size(); ++i)
                    {
                       _channelsBuffers[i] = allocateNewTempBuffer(&_channelsVec[i], len * sizeof(double));
                       _channelsViews[i] = _channelsBuffers[i].readOnlyslice<double>(0, len);
                    }
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
                  size_t read = 0;
                  auto idxBuffer = toUnderlying(outChannel);
                  auto outView = buf->slice<double>(0, toRead);
                  
                  while(!_unconsumedBuffers.at(idxBuffer).empty() && read < toRead)
                    {
                       double value = _unconsumedBuffers.at(idxBuffer).front();
                       outView[read++] = value;
                       _unconsumedBuffers.at(idxBuffer).pop_front();
                    }
                  if(read < toRead)
                    {
                       auto remainsToRead = toRead - read;
                       ensureChannelBuffersSizes(remainsToRead);
                       for(size_t i=0; i<_channels.size(); ++i)
                         {
                            auto& channel = _channels[i];
                            auto& tempChannelBuffer = _channelsBuffers[i];
                            channel->read(&tempChannelBuffer, remainsToRead * sizeof(double));
                         }
                       
                       for(size_t i=0; i<remainsToRead; ++i)
                         {
                            double sample = mix(idxBuffer, i);
                            outView[read++] = sample;
                            
                            sample = mix(1-idxBuffer, i);
                            _unconsumedBuffers.at(1-idxBuffer).push_back(sample);
                         }
                    }
               }
             
             namespace
               {
                  struct ChannelDescriptor
                    {
                       DepthPositions depthPosition;
                       bool left;
                       double defaultLeftCoefficient;
                       bool right;
                       double defaultRightCoefficient;
                    };
                  
                  std::array<ChannelDescriptor, toUnderlying(ChannelTypes::NbChannelTypes)> channelDescriptors 
                    {
                         {
                              { DepthPositions::Front, true, 1.0, false, 0.0 },         // FRONT_LEFT
                              { DepthPositions::Front, false, 0.0, true, 1.0 },         // FRONT_RIGHT
                              { DepthPositions::FrontCenter, true, 0.5, true, 0.5 },    // FRONT_CENTER
                              { DepthPositions::LowFrequency, true, 0.5, true, 0.5 },   // LOW_FREQUENCY
                              { DepthPositions::Back, true, 1.0, false, 0.0 },          // BACK_LEFT
                              { DepthPositions::Back, false, 0.0, true, 1.0 },          // BACK_RIGHT
                              { DepthPositions::FrontSide, true, 0.75, true, 0.25 },    // FRONT_LEFT_OF_CENTER
                              { DepthPositions::FrontSide, true, 0.25, true, 0.75 },    // FRONT_RIGHT_OF_CENTER
                              { DepthPositions::BackCenter, true, 0.5, true, 0.5 },     // BACK_CENTER
                              { DepthPositions::Side, true, 1.0, false, 0.0 },          // SIDE_LEFT
                              { DepthPositions::Side, false, 0.0, true, 1.0 },          // SIDE_RIGHT
                              { DepthPositions::TopCenter, true, 0.5, true, 0.5 },      // TOP_CENTER
                              { DepthPositions::TopFront, true, 0.75, true, 0.25 },     // TOP_FRONT_LEFT
                              { DepthPositions::TopFrontCenter, true, 0.5, true, 0.5 }, // TOP_FRONT_CENTER
                              { DepthPositions::TopFront, true, 0.25, true, 0.75 },     // TOP_FRONT_RIGHT
                              { DepthPositions::TopBack, true, 1.0, false, 0.0 },       // TOP_BACK_LEFT
                              { DepthPositions::TopBackCenter, true, 0.5, true, 0.5 },  // TOP_BACK_CENTER
                              { DepthPositions::TopBack, false, 0.0, true, 1.0 }        // TOP_ BACK_RIGHT
                         }
                    };
               } // namespace
             
             void InternalMultiChannelMixerSourceConverter::computeMixingCoefficients()
               {
                  u32 mask = 1;
                  size_t idxChannel =0;
                  constexpr int maxDepthPositions = toUnderlying(DepthPositions::NbDepthPositions);
                  std::array<bool, maxDepthPositions> filledDepthPositions {};
                  int nbDepthPositions = 0;
                  std::fill(filledDepthPositions.begin(), filledDepthPositions.end(), false);
                  
                  for(size_t i=0; i<2; ++i)
                    {
                       _coefficients.at(i).resize(_channels.size(), 0.0);
                    }
                  
                  for(auto& descriptor : channelDescriptors)
                    {
                       if((_channelMask & mask) != 0)
                         {
                            auto depthPosition = toUnderlying(descriptor.depthPosition);
                            if(!filledDepthPositions.at(depthPosition))
                              {
                                 ++nbDepthPositions;
                                 filledDepthPositions.at(depthPosition) = true;
                              }
                            if(descriptor.left) 
                              {
                                 _coefficients[toUnderlying(ChannelId::Left)][idxChannel] = descriptor.defaultLeftCoefficient;
                              }
                            if(descriptor.right)
                              {
                                 _coefficients[toUnderlying(ChannelId::Right)][idxChannel] = descriptor.defaultRightCoefficient;
                              }
                            ++idxChannel;
                            if(idxChannel >= _channels.size())
                              {
                                 break;
                              }
                         }
                       mask <<= 1u;
                    }
                  
                  // loop that assigns remaining channels
                  mask = 1;
                  for(auto& descriptor : channelDescriptors)
                    {
                       if(idxChannel >= _channels.size())
                         {
                            break;
                         }
                       if((_channelMask & mask) == 0) // free output
                         {
                            auto depthPosition = toUnderlying(descriptor.depthPosition);
                            if(!filledDepthPositions.at(depthPosition))
                              {
                                 ++nbDepthPositions;
                                 filledDepthPositions.at(depthPosition) = true;
                              }
                            if(descriptor.left) 
                              {
                                 _coefficients[toUnderlying(ChannelId::Left)][idxChannel] = descriptor.defaultLeftCoefficient;
                              }
                            if(descriptor.right)
                              {
                                 _coefficients[toUnderlying(ChannelId::Right)][idxChannel] = descriptor.defaultRightCoefficient;
                              }
                            ++idxChannel;
                         }
                       mask <<= 1u;
                    }
                  
                  for(size_t i=0; i<_channels.size(); ++i)
                    {
                       _coefficients[toUnderlying(ChannelId::Left)][i] /= nbDepthPositions;
                       _coefficients[toUnderlying(ChannelId::Right)][i] /= nbDepthPositions;
                    }
               }
             
             double InternalMultiChannelMixerSourceConverter::mix(int idxOutBuffer, size_t idxSample) const
               {
                  double sample = 0.0;
                  auto& coefficients = _coefficients.at(idxOutBuffer);
                  for(size_t i=0; i<_channels.size(); ++i)
                    {
                       auto& view = _channelsViews[i];
                       sample += coefficients[i] * view[idxSample];
                    }
                  return sample;
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
                         : MultiChannelMixerBase(src, ChannelId::Right)
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
        
        MultiChannelMixer::MultiChannelMixer(std::vector<WavConverter::ptr> channels, u32 channelMask)
          : MultiChannelMixerBase(std::make_shared<impl::InternalMultiChannelMixerSourceConverter>(std::move(channels), channelMask), impl::ChannelId::Left),
          _right(buildRightChannel())
            {
            }
        
        WavConverter::ptr MultiChannelMixer::getRightChannel()
          {
             return std::move(_right);
          }
     } // namespace wav
} // namespace mods