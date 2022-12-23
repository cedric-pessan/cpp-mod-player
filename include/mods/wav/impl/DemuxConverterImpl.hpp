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
             template<typename T>
               class InternalDemuxConverter
               {
                public:
                  using sptr = std::shared_ptr<InternalDemuxConverter>;
                  using Converter = mods::converters::Converter<T>;
                  
                  InternalDemuxConverter(typename Converter::ptr src, u32 nbChannels, u32 bitsPerContainer);
                  
                  InternalDemuxConverter() = delete;
                  InternalDemuxConverter(const InternalDemuxConverter&) = delete;
                  InternalDemuxConverter(InternalDemuxConverter&&) = delete;
                  auto operator=(const InternalDemuxConverter&) = delete;
                  auto operator=(InternalDemuxConverter&&) = delete;
                  ~InternalDemuxConverter() = default;
                  
                  auto isFinished(u32 channel) const -> bool;
                  void read(mods::utils::RWBuffer<T>* buf, u32 channel);
                  
                private:
                  auto allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>;
                  void ensureTempBufferSize(size_t len);
                  
                  using UnconsumedBuffer = mods::utils::DynamicRingBuffer<T>;
                  std::vector<UnconsumedBuffer> _unconsumedBuffers;
                  
                  typename Converter::ptr _src;
                  
                  u32 _elemsPerContainer;
                  u32 _nbChannels;
                  
                  std::vector<u8> _tempVec;
                  mods::utils::RWBuffer<u8> _temp;
               };
             
             template<typename T>
               class DemuxConverterSlave : public mods::converters::Converter<T>
               {
                public:
                  using ptr = typename mods::converters::Converter<T>::ptr;
                  
                  DemuxConverterSlave(typename InternalDemuxConverter<T>::sptr src, u32 channel);
                  
                  DemuxConverterSlave() = delete;
                  DemuxConverterSlave(const DemuxConverterSlave&) = delete;
                  DemuxConverterSlave(DemuxConverterSlave&&) = delete;
                  auto operator=(const DemuxConverterSlave&) = delete;
                  auto operator=(DemuxConverterSlave&&) = delete;
                  ~DemuxConverterSlave() override = default;
                  
                  auto isFinished() const -> bool override;
                  void read(mods::utils::RWBuffer<T>* buf) override;
                  
                protected:
                  auto buildSlaves(u32 nbChannels) const -> std::vector<ptr>;
                  
                private:
                  typename InternalDemuxConverter<T>::sptr _src;
                  u32 _channel;
               };
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_DEMUXCONVERTERIMPL_HPP
