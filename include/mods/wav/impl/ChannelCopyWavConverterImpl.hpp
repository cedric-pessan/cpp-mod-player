#ifndef MODS_WAV_IMPL_CHANNELCOPYWAVCONVERTERIMPL_HPP
#define MODS_WAV_IMPL_CHANNELCOPYWAVCONVERTERIMPL_HPP

#include "mods/utils/types.hpp"

#include <deque>

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
             
             class InternalSourceConverter
               {
                public:
                  using sptr = std::shared_ptr<InternalSourceConverter>;
                  
                  explicit InternalSourceConverter(WavConverter::ptr src);
                  
                  InternalSourceConverter() = delete;
                  InternalSourceConverter(const InternalSourceConverter&) = delete;
                  InternalSourceConverter(const InternalSourceConverter&&) = delete;
                  InternalSourceConverter& operator=(const InternalSourceConverter&) = delete;
                  InternalSourceConverter& operator=(const InternalSourceConverter&&) = delete;
                  ~InternalSourceConverter() = default;
                  
                  bool isFinished(CopyDestId id) const;
                  void read(mods::utils::RWBuffer<u8>* buf, int len, CopyDestId id);
                  
                private:
                  using UnconsumedBuffer = std::deque<u8>;
                  std::array<UnconsumedBuffer,2> _unconsumedBuffers;
                  
                  WavConverter::ptr _src;
               };
             
             class ChannelCopyWavConverterSlave : public WavConverter
               {
                protected:
                  ChannelCopyWavConverterSlave(InternalSourceConverter::sptr src, CopyDestId id);
                  
                public:
                  ChannelCopyWavConverterSlave() = delete;
                  ChannelCopyWavConverterSlave(const ChannelCopyWavConverterSlave&) = delete;
                  ChannelCopyWavConverterSlave(const ChannelCopyWavConverterSlave&&) = delete;
                  ChannelCopyWavConverterSlave& operator=(const ChannelCopyWavConverterSlave&) = delete;
                  ChannelCopyWavConverterSlave& operator=(const ChannelCopyWavConverterSlave&&) = delete;
                  ~ChannelCopyWavConverterSlave() override = default;
                  
                  bool isFinished() const override;
                  void read(mods::utils::RWBuffer<u8>* buf, int len) override;
                  
                protected:
                  WavConverter::ptr buildSlave() const;
             
                private:
                  InternalSourceConverter::sptr _src;
                  CopyDestId _id;
               };
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_CHANNELCOPYWAVCONVERTERIMPL_HPP
