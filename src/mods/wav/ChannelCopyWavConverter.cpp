
#include "mods/wav/ChannelCopyWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        namespace
          {
             /*class InternalSourceConverter : public WavConverter
               {
                public:
                  InternalSourceConverter();
                  InternalSourceConverter(const InternalSourceConverter&) = delete;
                  InternalSourceConverter(const InternalSourceConverter&&) = delete;
                  InternalSourceConverter& operator=(const InternalSourceConverter&) = delete;
                  InternalSourceConverter& operator=(const InternalSourceConverter&&) = delete;
                  ~InternalSourceConverter() override = default;
               };*/
          } // namespace
        
        WavConverter::ptr ChannelCopyWavConverterSlave::buildSlave() const
          {
             class make_unique_enabler : public ChannelCopyWavConverterSlave
               {
                public:
                  make_unique_enabler() = default;
                  make_unique_enabler(const make_unique_enabler&) = delete;
                  make_unique_enabler(const make_unique_enabler&&) = delete;
                  make_unique_enabler& operator=(const make_unique_enabler&) = delete;
                  make_unique_enabler& operator=(const make_unique_enabler&&) = delete;
                  ~make_unique_enabler() override = default;
               };
             return std::make_unique<make_unique_enabler>();
          }
        
        bool ChannelCopyWavConverterSlave::isFinished() const
          {
             std::cout << "TODO: ChannelCopyWavConverterSlave::isFinished() const" << std::endl;
             return false;
          }
        
        ChannelCopyWavConverter::ChannelCopyWavConverter()
          : /*_src(std::make_shared<InternalSourceConverter>()),*/
          _copy(buildSlave())
            {
            }
        
        WavConverter::ptr ChannelCopyWavConverter::getCopy()
          {
             return std::move(_copy);
          }
     } // namespace wav
} // namespace mods
