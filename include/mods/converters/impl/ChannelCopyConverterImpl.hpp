#ifndef MODS_CONVERTERS_IMPL_CHANNELCOPYCONVERTERIMPL_HPP
#define MODS_CONVERTERS_IMPL_CHANNELCOPYCONVERTERIMPL_HPP

#include "mods/converters/Converter.hpp"
#include "mods/utils/DynamicRingBuffer.hpp"

#include <array>

namespace mods
{
   namespace converters
     {
        namespace impl
          {
             enum struct CopyDestId : u8
               {
                  MASTER,
                    SLAVE
               };
             
             template<typename T>
               class InternalCopySourceConverter
               {
                public:
                  using sptr = std::shared_ptr<InternalCopySourceConverter>;
                  
                  explicit InternalCopySourceConverter(typename Converter<T>::ptr src);
                  
                  InternalCopySourceConverter() = delete;
                  InternalCopySourceConverter(const InternalCopySourceConverter&) = delete;
                  InternalCopySourceConverter(InternalCopySourceConverter&&) = delete;
                  auto operator=(const InternalCopySourceConverter&) -> InternalCopySourceConverter& = delete;
                  auto operator=(InternalCopySourceConverter&&) -> InternalCopySourceConverter& = delete;
                  ~InternalCopySourceConverter() = default;
                  
                  auto isFinished(CopyDestId copyId) const -> bool;
                  void read(mods::utils::RWBuffer<T>* buf, CopyDestId copyId);
                  
                private:
                  using UnconsumedBuffer = mods::utils::DynamicRingBuffer<T>;
                  std::array<UnconsumedBuffer,2> _unconsumedBuffers;
                  
                  typename Converter<T>::ptr _src;
               };
             
             template<typename T>
               class ChannelCopyConverterSlave : public Converter<T>
               {
                protected:
                  ChannelCopyConverterSlave(typename InternalCopySourceConverter<T>::sptr src, CopyDestId copyId);
                  
                public:
                  ChannelCopyConverterSlave() = delete;
                  ChannelCopyConverterSlave(const ChannelCopyConverterSlave&) = delete;
                  ChannelCopyConverterSlave(ChannelCopyConverterSlave&&) = delete;
                  auto operator=(const ChannelCopyConverterSlave&) -> ChannelCopyConverterSlave& = delete;
                  auto operator=(ChannelCopyConverterSlave&&) -> ChannelCopyConverterSlave& = delete;
                  ~ChannelCopyConverterSlave() override = default;
                  
                  auto isFinished() const -> bool override;
                  void read(mods::utils::RWBuffer<T>* buf) override;
                  
                protected:
                  auto buildSlave() const -> typename Converter<T>::ptr;
                  
                private:
                  typename InternalCopySourceConverter<T>::sptr _src;
                  CopyDestId _id;
               };
          } // namespace impl
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_IMPL_CHANNELCOPYCONVERTERIMPL_HPP
