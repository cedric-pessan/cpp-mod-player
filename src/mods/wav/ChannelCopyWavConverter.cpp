
#include "mods/wav/ChannelCopyWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        namespace
          {
             class InternalSourceConverter : public WavConverter
               {
                public:
                  InternalSourceConverter() = default;
                  InternalSourceConverter(const InternalSourceConverter&) = delete;
                  InternalSourceConverter(const InternalSourceConverter&&) = delete;
                  InternalSourceConverter& operator=(const InternalSourceConverter&) = delete;
                  InternalSourceConverter& operator=(const InternalSourceConverter&&) = delete;
                  ~InternalSourceConverter() override = default;
                  
                  bool isFinished() const override
                    {
                       std::cout << "TODO: ChannelCopy: InternalSourceConverter::isFinished() const" << std::endl;
                       return false;
                    }
               };
          } // namespace
        
        ChannelCopyWavConverterSlave::ChannelCopyWavConverterSlave(const WavConverter::sptr& src)
          : _src(src)
            {
            }
        
        WavConverter::ptr ChannelCopyWavConverterSlave::buildSlave() const
          {
             class make_unique_enabler : public ChannelCopyWavConverterSlave
               {
                public:
                  make_unique_enabler(const WavConverter::sptr& src)
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
        
        ChannelCopyWavConverter::ChannelCopyWavConverter()
          : ChannelCopyWavConverterSlave(std::make_shared<InternalSourceConverter>()),
          _copy(buildSlave())
            {
            }
        
        WavConverter::ptr ChannelCopyWavConverter::getCopy()
          {
             return std::move(_copy);
          }
     } // namespace wav
} // namespace mods
