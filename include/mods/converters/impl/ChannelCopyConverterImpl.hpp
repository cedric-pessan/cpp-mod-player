#ifndef MODS_CONVERTERS_IMPL_CHANNELCOPYCONVERTERIMPL_HPP
#define MODS_CONVERTERS_IMPL_CHANNELCOPYCONVERTERIMPL_HPP

#include "mods/converters/Converter.hpp"
#include "mods/utils/DynamicRingBuffer.hpp"

namespace mods
{
   namespace converters
     {
        namespace impl
          {
             enum struct CopyDestId
               {
                  MASTER,
                    SLAVE
               };
             
             class InternalCopySourceConverter
               {
                public:
                  using sptr = std::shared_ptr<InternalCopySourceConverter>;
                  
                  explicit InternalCopySourceConverter(Converter::ptr src);
                  
                  InternalCopySourceConverter() = delete;
                  InternalCopySourceConverter(const InternalCopySourceConverter&) = delete;
                  InternalCopySourceConverter(InternalCopySourceConverter&&) = delete;
                  auto operator=(const InternalCopySourceConverter&) -> InternalCopySourceConverter& = delete;
                  auto operator=(InternalCopySourceConverter&&) -> InternalCopySourceConverter& = delete;
                  ~InternalCopySourceConverter() = default;
                  
                  auto isFinished(CopyDestId id) const -> bool;
                  void read(mods::utils::RWBuffer<u8>* buf, size_t len, CopyDestId id);
                  
                private:
                  using UnconsumedBuffer = mods::utils::DynamicRingBuffer<u8>;
                  std::array<UnconsumedBuffer,2> _unconsumedBuffers;
                  
                  Converter::ptr _src;
               };
             
             class ChannelCopyConverterSlave : public Converter
               {
                protected:
                  ChannelCopyConverterSlave(InternalCopySourceConverter::sptr src, CopyDestId id);
                  
                public:
                  ChannelCopyConverterSlave() = delete;
                  ChannelCopyConverterSlave(const ChannelCopyConverterSlave&) = delete;
                  ChannelCopyConverterSlave(ChannelCopyConverterSlave&&) = delete;
                  auto operator=(const ChannelCopyConverterSlave&) -> ChannelCopyConverterSlave& = delete;
                  auto operator=(ChannelCopyConverterSlave&&) -> ChannelCopyConverterSlave& = delete;
                  ~ChannelCopyConverterSlave() override = default;
                  
                  auto isFinished() const -> bool override;
                  void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
                  
                protected:
                  auto buildSlave() const -> Converter::ptr;
                  
                private:
                  InternalCopySourceConverter::sptr _src;
                  CopyDestId _id;
               };
          } // namespace impl
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_IMPL_CHANNELCOPYCONVERTERIMPL_HPP
