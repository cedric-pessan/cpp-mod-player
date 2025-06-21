
#include "mods/converters/Converter.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/RWBufferBackend.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/MultiChannelMixer.hpp"
#include "mods/wav/impl/MultiChannelMixerImpl.hpp"

#include <array>
#include <cstdlib>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             InternalMultiChannelMixerSourceConverter::InternalMultiChannelMixerSourceConverter(std::vector<Converter::ptr> channels, u32 channelMask)
               : _channels(std::move(channels))
                 {
                    for(size_t i = 0; i < _channels.size(); ++i)
                      {
                         _channelsVec.emplace_back();
                         _channelsBuffers.emplace_back(allocateNewTempBuffer(&_channelsVec.back(), 0));
                      }
                    
                    computeMixingCoefficients(channelMask);
                 }
             
             auto InternalMultiChannelMixerSourceConverter::allocateNewTempBuffer(std::vector<u8>* backendVec, size_t len) -> mods::utils::RWBuffer<double>
               {
                  backendVec->resize(len * sizeof(double));
                  u8* ptr = backendVec->data();
                  auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
                  auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, len * sizeof(double), std::move(deleter));
                  return mods::utils::RWBuffer<double>(std::move(buffer));
               }
             
             void InternalMultiChannelMixerSourceConverter::ensureChannelBuffersSizes(size_t len)
               {
                  for(size_t i=0; i<_channels.size(); ++i)
                    {
                       _channelsBuffers[i] = allocateNewTempBuffer(&_channelsVec[i], len);
                    }
               }
             
             auto InternalMultiChannelMixerSourceConverter::isFinished(ChannelId outChannel) const -> bool
               {
                  auto idxBuffer = toUnderlying(outChannel);
                  if(!_unconsumedBuffers.at(idxBuffer).empty())
                    {
                       return false;
                    }
                  for(const auto& channel : _channels) 
                    {
                       if(!channel->isFinished())
                         {
                            return false;
                         }
                    }
                  return true;
               }
             
             void InternalMultiChannelMixerSourceConverter::read(mods::utils::RWBuffer<double>* buf, ChannelId outChannel)
               {
                  const size_t toRead = buf->size();
                  size_t read = 0;
                  auto idxBuffer = toUnderlying(outChannel);
                  auto& outView = *buf;
                  
                  while(!_unconsumedBuffers.at(idxBuffer).empty() && read < toRead)
                    {
                       const double value = _unconsumedBuffers.at(idxBuffer).front();
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
                            auto tempChannelBuffer = _channelsBuffers[i].slice<double>(0, remainsToRead);;
                            channel->read(&tempChannelBuffer);
                         }
                       
                       for(size_t i=0; i<remainsToRead; ++i)
                         {
                            double sample = mix(_coefficients.at(idxBuffer), i);
                            outView[read++] = sample;
                            
                            sample = mix(_coefficients.at(1-idxBuffer), i);
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
                  
                  constexpr std::array<ChannelDescriptor, toUnderlying(ChannelTypes::NbChannelTypes)> channelDescriptors
                    {
                         {
                              { DepthPositions::Front,          true,  1.0,  false, 0.0  }, // FRONT_LEFT
                              { DepthPositions::Front,          false, 0.0,  true,  1.0  }, // FRONT_RIGHT
                              { DepthPositions::FrontCenter,    true,  0.5,  true,  0.5  }, // FRONT_CENTER
                              { DepthPositions::LowFrequency,   true,  0.5,  true,  0.5  }, // LOW_FREQUENCY
                              { DepthPositions::Back,           true,  1.0,  false, 0.0  }, // BACK_LEFT
                              { DepthPositions::Back,           false, 0.0,  true,  1.0  }, // BACK_RIGHT
                              { DepthPositions::FrontSide,      true,  0.75, true,  0.25 }, // FRONT_LEFT_OF_CENTER
                              { DepthPositions::FrontSide,      true,  0.25, true,  0.75 }, // FRONT_RIGHT_OF_CENTER
                              { DepthPositions::BackCenter,     true,  0.5,  true,  0.5  }, // BACK_CENTER
                              { DepthPositions::Side,           true,  1.0,  false, 0.0  }, // SIDE_LEFT
                              { DepthPositions::Side,           false, 0.0,  true,  1.0  }, // SIDE_RIGHT
                              { DepthPositions::TopCenter,      true,  0.5,  true,  0.5  }, // TOP_CENTER
                              { DepthPositions::TopFront,       true,  0.75, true,  0.25 }, // TOP_FRONT_LEFT
                              { DepthPositions::TopFrontCenter, true,  0.5,  true,  0.5  }, // TOP_FRONT_CENTER
                              { DepthPositions::TopFront,       true,  0.25, true,  0.75 }, // TOP_FRONT_RIGHT
                              { DepthPositions::TopBack,        true,  1.0,  false, 0.0  }, // TOP_BACK_LEFT
                              { DepthPositions::TopBackCenter,  true,  0.5,  true,  0.5  }, // TOP_BACK_CENTER
                              { DepthPositions::TopBack,        false, 0.0,  true,  1.0  }  // TOP_ BACK_RIGHT
                         }
                    };
               } // namespace
             
             void InternalMultiChannelMixerSourceConverter::computeMixingCoefficients(u32 channelMask)
               {
                  u32 mask = 1;
                  size_t idxChannel =0;
                  std::unordered_set<DepthPositions> filledDepthPositions;
                  
                  for(size_t i=0; i<2; ++i)
                    {
                       _coefficients.at(i).resize(_channels.size(), 0.0);
                    }
                  
                  for(const auto& descriptor : channelDescriptors)
                    {
                       if((channelMask & mask) != 0)
                         {
                            filledDepthPositions.insert(descriptor.depthPosition);
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
                       mask <<= 1U;
                    }
                  
                  // loop that assigns remaining channels
                  mask = 1;
                  for(const auto& descriptor : channelDescriptors)
                    {
                       if(idxChannel >= _channels.size())
                         {
                            break;
                         }
                       if((channelMask & mask) == 0) // free output
                         {
                            filledDepthPositions.insert(descriptor.depthPosition);
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
                       mask <<= 1U;
                    }
                  
                  for(size_t i=0; i<_channels.size(); ++i)
                    {
                       _coefficients[toUnderlying(ChannelId::Left)][i] /= static_cast<double>(filledDepthPositions.size());
                       _coefficients[toUnderlying(ChannelId::Right)][i] /= static_cast<double>(filledDepthPositions.size());
                    }
               }
             
             auto InternalMultiChannelMixerSourceConverter::mix(const std::vector<double>& coefficients, size_t idxSample) const -> double
               {
                  double sample = 0.0;
                  for(size_t i=0; i<_channels.size(); ++i)
                    {
                       const auto& buf = _channelsBuffers[i];
                       sample += coefficients[i] * buf[idxSample];
                    }
                  return sample;
               }
             
             MultiChannelMixerBase::MultiChannelMixerBase(InternalMultiChannelMixerSourceConverter::sptr src, ChannelId channel)
               : _src(std::move(src)),
               _channel(channel)
                 {
                 }
             
             auto MultiChannelMixerBase::buildRightChannel() const -> Converter::ptr
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
                       auto operator=(const make_unique_enabler&) -> make_unique_enabler& = delete;
                       auto operator=(make_unique_enabler&&) -> make_unique_enabler& = delete;
                       ~make_unique_enabler() override = default;
                    };
                  return std::make_unique<make_unique_enabler>(_src);
               }
             
             auto MultiChannelMixerBase::isFinished() const -> bool
               {
                  return _src->isFinished(_channel);
               }
             
             void MultiChannelMixerBase::read(mods::utils::RWBuffer<double>* buf)
               {
                  _src->read(buf, _channel);
               }
          } // namespace impl
        
        MultiChannelMixer::MultiChannelMixer(std::vector<Converter::ptr> channels, u32 channelMask)
          : MultiChannelMixerBase(std::make_unique<impl::InternalMultiChannelMixerSourceConverter>(std::move(channels), channelMask), impl::ChannelId::Left),
          _right(buildRightChannel())
            {
            }
        
        auto MultiChannelMixer::getRightChannel() -> Converter<double>::ptr
          {
             return std::move(_right);
          }
     } // namespace wav
} // namespace mods
