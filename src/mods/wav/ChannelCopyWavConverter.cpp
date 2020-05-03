
#include "mods/wav/ChannelCopyWavConverter.hpp"

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             InternalCopySourceConverter::InternalCopySourceConverter(WavConverter::ptr src)
               : _src(std::move(src))
                 {
                 }
             
             auto InternalCopySourceConverter::isFinished(CopyDestId id) const -> bool
               {
                  auto idxBuffer = toUnderlying(id);
                  if(!_unconsumedBuffers.at(idxBuffer).empty())
                    {
                       return false;
                    }
                  return _src->isFinished();
               }
             
             void InternalCopySourceConverter::read(mods::utils::RWBuffer<u8>* buf, int len, CopyDestId id)
               {
                  int read = 0;
                  auto idxBuffer = toUnderlying(id);
                  auto& out = *buf;
                  while(!_unconsumedBuffers.at(idxBuffer).empty() && read < len)
                    {
                       u8 value = _unconsumedBuffers.at(idxBuffer).front();
                       out[read++] = value;
                       _unconsumedBuffers.at(idxBuffer).pop_front();
                    }
                  if(read < len)
                    {
                       auto remainingBuffer = buf->slice<u8>(read, len-read);
                       _src->read(&remainingBuffer, len - read);
                       for(int i=0; i < len-read; ++i)
                         {
                            _unconsumedBuffers.at(1-idxBuffer).push_back(remainingBuffer[i]);
                         }
                    }
               }
             
             ChannelCopyWavConverterSlave::ChannelCopyWavConverterSlave(InternalCopySourceConverter::sptr src, CopyDestId id)
               : _src(std::move(src)),
               _id(id)
                 {
                 }
             
             auto ChannelCopyWavConverterSlave::buildSlave() const -> WavConverter::ptr
               {
                  class make_unique_enabler : public ChannelCopyWavConverterSlave
                    {
                     public:
                       explicit make_unique_enabler(const InternalCopySourceConverter::sptr& src)
                         : ChannelCopyWavConverterSlave(src, CopyDestId::SLAVE)
                           {
                           }
                       
                       make_unique_enabler() = delete;
                       make_unique_enabler(const make_unique_enabler&) = delete;
                       make_unique_enabler(make_unique_enabler&&) = delete;
                       auto operator=(const make_unique_enabler&) -> make_unique_enabler& = delete;
                       auto operator=(make_unique_enabler&&) -> make_unique_enabler& = delete;
                       ~make_unique_enabler() override = default;
                    };
                  return std::make_unique<make_unique_enabler>(_src);
               }
             
             auto ChannelCopyWavConverterSlave::isFinished() const -> bool
               {
                  return _src->isFinished(_id);
               }
             
             void ChannelCopyWavConverterSlave::read(mods::utils::RWBuffer<u8>* buf, size_t len)
               {
                  _src->read(buf, len, _id);
               }
          } // namespace impl
        
        ChannelCopyWavConverter::ChannelCopyWavConverter(WavConverter::ptr src)
          : ChannelCopyWavConverterSlave(std::make_shared<impl::InternalCopySourceConverter>(std::move(src)), impl::CopyDestId::MASTER),
          _copy(buildSlave())
            {
            }
        
        auto ChannelCopyWavConverter::getCopy() -> WavConverter::ptr
          {
             return std::move(_copy);
          }
     } // namespace wav
} // namespace mods
