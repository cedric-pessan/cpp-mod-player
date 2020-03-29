#ifndef MODS_WAV_IMPL_CHANNELCOPYWAVCONVERTERIMPL_HPP
#define MODS_WAV_IMPL_CHANNELCOPYWAVCONVERTERIMPL_HPP

#include "mods/wav/WavConverter.hpp"

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
             
             class InternalCopySourceConverter
               {
                public:
                  using sptr = std::shared_ptr<InternalCopySourceConverter>;
                  
                  explicit InternalCopySourceConverter(WavConverter::ptr src);
                  
                  InternalCopySourceConverter() = delete;
                  InternalCopySourceConverter(const InternalCopySourceConverter&) = delete;
                  InternalCopySourceConverter(InternalCopySourceConverter&&) = delete;
                  InternalCopySourceConverter& operator=(const InternalCopySourceConverter&) = delete;
                  InternalCopySourceConverter& operator=(InternalCopySourceConverter&&) = delete;
                  ~InternalCopySourceConverter() = default;
                  
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
                  ChannelCopyWavConverterSlave(InternalCopySourceConverter::sptr src, CopyDestId id);
                  
                public:
                  ChannelCopyWavConverterSlave() = delete;
                  ChannelCopyWavConverterSlave(const ChannelCopyWavConverterSlave&) = delete;
                  ChannelCopyWavConverterSlave(ChannelCopyWavConverterSlave&&) = delete;
                  ChannelCopyWavConverterSlave& operator=(const ChannelCopyWavConverterSlave&) = delete;
                  ChannelCopyWavConverterSlave& operator=(ChannelCopyWavConverterSlave&&) = delete;
                  ~ChannelCopyWavConverterSlave() override = default;
                  
                  bool isFinished() const override;
                  void read(mods::utils::RWBuffer<u8>* buf, int len) override;
                  
                protected:
                  WavConverter::ptr buildSlave() const;
                  
                private:
                  InternalCopySourceConverter::sptr _src;
                  CopyDestId _id;
               };
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_CHANNELCOPYWAVCONVERTERIMPL_HPP
