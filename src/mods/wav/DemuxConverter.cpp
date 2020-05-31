
#include "mods/wav/DemuxConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             InternalDemuxConverter::InternalDemuxConverter(WavConverter::ptr src, u32 nbChannels, u32 bitsPerContainer)
               : _unconsumedBuffers(nbChannels),
               _src(std::move(src)),
               _bytesPerContainer(bitsPerContainer / BITS_IN_BYTE),
               _nbChannels(nbChannels),
               _temp(allocateNewTempBuffer(0))
                 {
                 }
             
             auto InternalDemuxConverter::allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>
               {
                  _tempVec.resize(len);
                  u8* ptr = _tempVec.data();
                  auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
                  auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
                  return mods::utils::RWBuffer<u8>(buffer);
               }
             
             void InternalDemuxConverter::ensureTempBufferSize(size_t len)
               {
                  if(_temp.size() < len)
                    {
                       _temp = allocateNewTempBuffer(len);
                    }
               }
             
             auto InternalDemuxConverter::isFinished(u32 channel) const -> bool
               {
                  if(!_unconsumedBuffers[channel].empty())
                    {
                       return false;
                    }
                  return _src->isFinished();
               }
             
             void InternalDemuxConverter::read(mods::utils::RWBuffer<u8>* buf, size_t len, u32 channel)
               {
                  if((len % _bytesPerContainer) != 0)
                    {
                       std::cout << "Error: length should be a multiple of container size" << std::endl;
                    }
                  
                  size_t read = 0;
                  auto& out = *buf;
                  
                  while(!_unconsumedBuffers[channel].empty() && read < len)
                    {
                       auto& unconsumed = _unconsumedBuffers[channel];
                       u8 value = unconsumed.front();
                       out[read++] = value;
                       unconsumed.pop_front();
                    }
                  if(read < len)
                    {
                       auto bytesToRead = len - read;
                       auto toReadMuxed = bytesToRead * _nbChannels;
                       ensureTempBufferSize(toReadMuxed);
                       _src->read(&_temp, toReadMuxed);
                       
                       u32 currentChannel = 0;
                       u32 currentByteInContainer = 0;
                       
                       for(size_t i=0; i<toReadMuxed; ++i)
                         {
                            u8 value = _temp[i];
                            if(currentChannel == channel)
                              {
                                 out[read++] = value;
                              }
                            else
                              {
                                 _unconsumedBuffers[currentChannel].push_back(value);
                              }
                            currentByteInContainer++;
                            if(currentByteInContainer == _bytesPerContainer)
                              {
                                 currentByteInContainer = 0;
                                 ++currentChannel;
                                 if(currentChannel == _nbChannels)
                                   {
                                      currentChannel = 0;
                                   }
                              }
                         }
                    }
               }
             
             DemuxConverterSlave::DemuxConverterSlave(InternalDemuxConverter::sptr src, u32 channel)
               : _src(std::move(src)),
               _channel(channel)
                 {
                 }
             
             auto DemuxConverterSlave::buildSlaves(u32 nbChannels) const -> std::vector<ptr>
               {
                  class make_unique_enabler : public DemuxConverterSlave
                    {
                     public:
                       make_unique_enabler(const InternalDemuxConverter::sptr& src, u32 channel)
                         : DemuxConverterSlave(src, channel)
                           {
                           }
                       
                       make_unique_enabler() = delete;
                       make_unique_enabler(const make_unique_enabler&) = delete;
                       make_unique_enabler(make_unique_enabler&&) = delete;
                       auto operator=(const make_unique_enabler&) -> make_unique_enabler& = delete;
                       auto operator=(make_unique_enabler&&) -> make_unique_enabler& = delete;
                       ~make_unique_enabler() override = default;
                    };
                  
                  std::vector<ptr> slaves;
                  for(u32 i=0; i<nbChannels-1; ++i)
                    {
                       slaves.push_back(std::make_unique<make_unique_enabler>(_src, i));
                    }
                  return slaves;
               }
             
             auto DemuxConverterSlave::isFinished() const -> bool
               {
                  return _src->isFinished(_channel);
               }
             
             void DemuxConverterSlave::read(mods::utils::RWBuffer<u8>* buf, size_t len)
               {
                  _src->read(buf, len, _channel);
               }
          } // namespace impl
        
        DemuxConverter::DemuxConverter(WavConverter::ptr src, u32 nbChannels, u32 bitsPerContainer)
          : DemuxConverterSlave(std::make_shared<impl::InternalDemuxConverter>(std::move(src), nbChannels, bitsPerContainer), nbChannels-1),
          _firstChannels(buildSlaves(nbChannels))
            {
            }
        
        auto DemuxConverter::getFirstChannels() -> std::vector<ptr>*
          {
             return &_firstChannels;
          }
     } // namespace wav
} // namespace mods
