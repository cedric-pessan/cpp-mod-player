
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/RWBufferBackend.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/DemuxConverter.hpp"
#include "mods/wav/impl/DemuxConverterImpl.hpp"

#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             template<typename T>
               InternalDemuxConverter<T>::InternalDemuxConverter(u32 nbChannels, typename Converter::ptr src, u32 bitsPerContainer)
                 : _unconsumedBuffers(nbChannels),
               _src(std::move(src)),
               _elemsPerContainer(bitsPerContainer / BITS_IN_BYTE / sizeof(T)),
               _nbChannels(nbChannels),
               _temp(allocateNewTempBuffer(0))
                 {
                    if(_elemsPerContainer * sizeof(T) * BITS_IN_BYTE != bitsPerContainer)
                      {
                         std::cout << "Error: " << bitsPerContainer << " bits per container doesn't container an integer number of elements of type " << sizeof(T) << '\n';
                      }
                 }
             
             template<typename T>
               auto InternalDemuxConverter<T>::allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>
               {
                  _tempVec.resize(len);
                  u8* ptr = _tempVec.data();
                  auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
                  auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
                  return mods::utils::RWBuffer<u8>(std::move(buffer));
               }
             
             template<typename T>
               void InternalDemuxConverter<T>::ensureTempBufferSize(size_t len)
                 {
                    if(_temp.size() < len)
                      {
                         _temp = allocateNewTempBuffer(len);
                      }
                 }
             
             template<typename T>
               auto InternalDemuxConverter<T>::isFinished(u32 channel) const -> bool
               {
                  if(!_unconsumedBuffers[channel].empty())
                    {
                       return false;
                    }
                  return _src->isFinished();
               }
             
             template<typename T>
               void InternalDemuxConverter<T>::read(mods::utils::RWBuffer<T>* buf, u32 channel)
                 {
                    if((buf->size() % _elemsPerContainer) != 0)
                      {
                         std::cout << "Error: length should be a multiple of container size" << '\n';
                      }
                    
                    size_t read = 0;
                    auto& out = *buf;
                    
                    while(!_unconsumedBuffers[channel].empty() && read < out.size())
                      {
                         auto& unconsumed = _unconsumedBuffers[channel];
                         auto value = unconsumed.front();
                         out[read++] = value;
                         unconsumed.pop_front();
                      }
                    if(read < out.size())
                      {
                         auto elemsToRead = out.size() - read;
                         auto toReadMuxed = elemsToRead * _nbChannels;
                         ensureTempBufferSize(toReadMuxed * sizeof(T));
                         auto tempView = _temp.slice<T>(0, toReadMuxed);
                         _src->read(&tempView);
                         
                         u32 currentChannel = 0;
                         u32 currentElemInContainer = 0;
                         
                         for(size_t i=0; i<toReadMuxed; ++i)
                           {
                              auto value = tempView[i];
                              if(currentChannel == channel)
                                {
                                   out[read++] = value;
                                }
                              else
                                {
                                   _unconsumedBuffers[currentChannel].push_back(value);
                                }
                              currentElemInContainer++;
                              if(currentElemInContainer == _elemsPerContainer)
                                {
                                   currentElemInContainer = 0;
                                   ++currentChannel;
                                   if(currentChannel == _nbChannels)
                                     {
                                        currentChannel = 0;
                                     }
                                }
                           }
                      }
                 }
             
             template<typename T>
               DemuxConverterSlave<T>::DemuxConverterSlave(typename InternalDemuxConverter<T>::sptr src, u32 channel)
                 : _src(std::move(src)),
               _channel(channel)
                 {
                 }
             
             template<typename T>
               auto DemuxConverterSlave<T>::buildSlaves(u32 nbChannels) const -> std::vector<ptr>
               {
                  class make_unique_enabler : public DemuxConverterSlave<T>
                    {
                     public:
                       make_unique_enabler(const typename InternalDemuxConverter<T>::sptr& src, u32 channel)
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
                  slaves.reserve(nbChannels-1);
                  for(u32 i=0; i<nbChannels-1; ++i)
                    {
                       slaves.push_back(std::make_unique<make_unique_enabler>(_src, i));
                    }
                  return slaves;
               }
             
             template<typename T>
               auto DemuxConverterSlave<T>::isFinished() const -> bool
               {
                  return _src->isFinished(_channel);
               }
             
             template<typename T>
               void DemuxConverterSlave<T>::read(mods::utils::RWBuffer<T>* buf)
               {
                  _src->read(buf, _channel);
               }
          } // namespace impl
        
        template<typename T>
          DemuxConverter<T>::DemuxConverter(ptr src, u32 nbChannels, u32 bitsPerContainer)
            : impl::DemuxConverterSlave<T>(std::make_unique<impl::InternalDemuxConverter<T>>(nbChannels, std::move(src), bitsPerContainer), nbChannels-1),
          _firstChannels(impl::DemuxConverterSlave<T>::buildSlaves(nbChannels))
            {
            }
        
        template<typename T>
          auto DemuxConverter<T>::getFirstChannels() -> std::vector<ptr>*
          {
             return &_firstChannels;
          }
        
        template class DemuxConverter<s8>;
        template class DemuxConverter<u8>;
        template class DemuxConverter<s16>;
     } // namespace wav
} // namespace mods
