
#include "mods/wav/ChannelCopyWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             InternalSourceConverter::InternalSourceConverter(WavConverter::ptr src)
               : _src(std::move(src))
                 {
                 }
             
             bool InternalSourceConverter::isFinished(CopyDestId id) const
               {
                  auto idxBuffer = toUnderlying(id);
                  if(!_unconsumedBuffers[idxBuffer].empty())
                    {
                       return false;
                    }
                  return _src->isFinished();
               }
             
             void InternalSourceConverter::read(mods::utils::RWBuffer<u8>& buf, int len)
               {
                  _src->read(buf, len);
                  std::cout << "TODO: InternalSourceConverter::read()" << std::endl;
               }
             
             ChannelCopyWavConverterSlave::ChannelCopyWavConverterSlave(const InternalSourceConverter::sptr& src, CopyDestId id)
               : _src(src),
               _id(id)
                 {
                 }
        
             WavConverter::ptr ChannelCopyWavConverterSlave::buildSlave() const
               {
                  class make_unique_enabler : public ChannelCopyWavConverterSlave
                    {
                     public:
                       make_unique_enabler(const InternalSourceConverter::sptr& src)
                         : ChannelCopyWavConverterSlave(src, CopyDestId::SLAVE)
                           {
                           }
                       
                       make_unique_enabler() = delete;
                       make_unique_enabler(const make_unique_enabler&) = delete;
                       make_unique_enabler(const make_unique_enabler&&) = delete;
                       make_unique_enabler& operator=(const make_unique_enabler&) = delete;
                       make_unique_enabler& operator=(const make_unique_enabler&&) = delete;
                       ~make_unique_enabler() override = default;
                    };
                  return std::make_unique<make_unique_enabler>(_src);
               }
        
             bool ChannelCopyWavConverterSlave::isFinished() const
               {
                  return _src->isFinished(_id);
               }
             
             void ChannelCopyWavConverterSlave::read(mods::utils::RWBuffer<u8>& buf, int len)
               {
                  _src->read(buf, len);
                  std::cout << "TODO: ChannelCopyWavConverterSlave::read()" << std::endl;
               }
          } // namespace impl
        
        ChannelCopyWavConverter::ChannelCopyWavConverter(WavConverter::ptr src)
          : ChannelCopyWavConverterSlave(std::make_shared<impl::InternalSourceConverter>(std::move(src)), impl::CopyDestId::MASTER),
          _copy(buildSlave())
            {
            }
        
        WavConverter::ptr ChannelCopyWavConverter::getCopy()
          {
             return std::move(_copy);
          }
     } // namespace wav
} // namespace mods
