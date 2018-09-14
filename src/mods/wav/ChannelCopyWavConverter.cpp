
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
             
             class InternalCopyWavConverter : public WavConverter
               {
                public:
                  InternalCopyWavConverter() = default;
                  InternalCopyWavConverter(const InternalCopyWavConverter&) = delete;
                  InternalCopyWavConverter(const InternalCopyWavConverter&&) = delete;
                  InternalCopyWavConverter& operator=(const InternalCopyWavConverter&) = delete;
                  InternalCopyWavConverter& operator=(const InternalCopyWavConverter&&) = delete;
                  ~InternalCopyWavConverter() override = default;
                  
                  bool isFinished() const override
                    {
                       std::cout << "TODO: InternalCopyWavConverter::isFinished() const" << std::endl;
                       return false;
                    }
               };
          } // namespace
        
        ChannelCopyWavConverter::ChannelCopyWavConverter()
          : /*_src(std::make_shared<InternalSourceConverter>()),*/
          _copy(std::make_unique<InternalCopyWavConverter>())
            {
            }
        
        bool ChannelCopyWavConverter::isFinished() const
          {
             std::cout << "TODO: ChannelCopyWavConverter::isFinished() const" << std::endl;
             return false;
          }
        
        WavConverter::ptr ChannelCopyWavConverter::getCopy()
          {
             return std::move(_copy);
          }
     } // namespace wav
} // namespace mods
