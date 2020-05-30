#ifndef MODS_WAV_IMPL_CHANNELCOPYWAVCONVERTERIMPL_HPP
#define MODS_WAV_IMPL_CHANNELCOPYWAVCONVERTERIMPL_HPP

#include "mods/utils/DynamicRingBuffer.hpp"
#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
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
                  
                  explicit InternalCopySourceConverter(WavConverter::ptr src);
                  
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
                  
                  WavConverter::ptr _src;
               };
             
             class ChannelCopyWavConverterSlave : public WavConverter
               {
                protected:
                  ChannelCopyWavConverterSlave(InternalCopySourceConverter::sptr src, CopyDestId id);
                  
                public:
                  ChannelCopyWavConverterSlave() = delete;
                  ChannelCopyWavConverterSlave(const ChannelCopyWavConverterSlave&) = delete;
                  ChannelCopyWavConverterSlave(ChannelCopyWavConverterSlave&&) = delete;
                  auto operator=(const ChannelCopyWavConverterSlave&) -> ChannelCopyWavConverterSlave& = delete;
                  auto operator=(ChannelCopyWavConverterSlave&&) -> ChannelCopyWavConverterSlave& = delete;
                  ~ChannelCopyWavConverterSlave() override = default;
                  
                  auto isFinished() const -> bool override;
                  void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
                  
                protected:
                  auto buildSlave() const -> WavConverter::ptr;
                  
                private:
                  InternalCopySourceConverter::sptr _src;
                  CopyDestId _id;
               };
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_CHANNELCOPYWAVCONVERTERIMPL_HPP
