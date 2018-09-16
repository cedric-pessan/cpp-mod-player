
#include "mods/wav/ChannelCopyWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             bool InternalSourceConverter::isFinished() const
               {
                  std::cout << "TODO: InternalSourceConverter::isFinished() const" << std::endl;
                  return false;
               }
             
             ChannelCopyWavConverterSlave::ChannelCopyWavConverterSlave(const InternalSourceConverter::sptr& src)
               : _src(src)
                 {
                 }
        
             WavConverter::ptr ChannelCopyWavConverterSlave::buildSlave() const
               {
                  class make_unique_enabler : public ChannelCopyWavConverterSlave
                    {
                     public:
                       make_unique_enabler(const InternalSourceConverter::sptr& src)
                         : ChannelCopyWavConverterSlave(src)
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
                  return _src->isFinished();
               }
          } // namespace impl
        
        ChannelCopyWavConverter::ChannelCopyWavConverter()
          : ChannelCopyWavConverterSlave(std::make_shared<impl::InternalSourceConverter>()),
          _copy(buildSlave())
            {
            }
        
        WavConverter::ptr ChannelCopyWavConverter::getCopy()
          {
             return std::move(_copy);
          }
     } // namespace wav
} // namespace mods
