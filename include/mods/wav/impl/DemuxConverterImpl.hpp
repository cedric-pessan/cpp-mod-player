#ifndef MODS_WAV_IMPL_DEMUXCONVERTERIMPL_HPP
#define MODS_WAV_IMPL_DEMUXCONVERTERIMPL_HPP

#include "mods/converters/Converter.hpp"
#include "mods/utils/DynamicRingBuffer.hpp"

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             class InternalDemuxConverter
               {
                public:
                  using sptr = std::shared_ptr<InternalDemuxConverter>;
                  using Converter = mods::converters::Converter;
                  
                  explicit InternalDemuxConverter(Converter::ptr src, u32 nbChannels, u32 bitsPerContainer);
                  
                  InternalDemuxConverter() = delete;
                  InternalDemuxConverter(const InternalDemuxConverter&) = delete;
                  InternalDemuxConverter(InternalDemuxConverter&&) = delete;
                  auto operator=(const InternalDemuxConverter&) = delete;
                  auto operator=(InternalDemuxConverter&&) = delete;
                  ~InternalDemuxConverter() = default;
                  
                  auto isFinished(u32 channel) const -> bool;
                  void read(mods::utils::RWBuffer<u8>* buf, size_t len, u32 channel);
                  
                private:
                  auto allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>;
                  void ensureTempBufferSize(size_t len);
                  
                  using UnconsumedBuffer = mods::utils::DynamicRingBuffer<u8>;
                  std::vector<UnconsumedBuffer> _unconsumedBuffers;
                  
                  Converter::ptr _src;
                  
                  u32 _bytesPerContainer;
                  u32 _nbChannels;
                  
                  std::vector<u8> _tempVec;
                  mods::utils::RWBuffer<u8> _temp;
               };
             
             class DemuxConverterSlave : public mods::converters::Converter
               {
                public:
                  DemuxConverterSlave(InternalDemuxConverter::sptr src, u32 channel);
                  
                  DemuxConverterSlave() = delete;
                  DemuxConverterSlave(const DemuxConverterSlave&) = delete;
                  DemuxConverterSlave(DemuxConverterSlave&&) = delete;
                  auto operator=(const DemuxConverterSlave&) = delete;
                  auto operator=(DemuxConverterSlave&&) = delete;
                  ~DemuxConverterSlave() override = default;
                  
                  auto isFinished() const -> bool override;
                  void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
                  
                protected:
                  auto buildSlaves(u32 nbChannels) const -> std::vector<ptr>;
                  
                private:
                  InternalDemuxConverter::sptr _src;
                  u32 _channel;
               };
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_DEMUXCONVERTERIMPL_HPP
