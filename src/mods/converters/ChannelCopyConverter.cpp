
#include "mods/converters/ChannelCopyConverter.hpp"
#include "mods/converters/Converter.hpp"
#include "mods/converters/impl/ChannelCopyConverterImpl.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"

#include <cstddef>

namespace mods
{
   namespace converters
     {
        namespace impl
          {
             template<typename T>
               InternalCopySourceConverter<T>::InternalCopySourceConverter(typename Converter<T>::ptr src)
                 : _src(std::move(src))
                   {
                   }
             
             template<typename T>
               auto InternalCopySourceConverter<T>::isFinished(CopyDestId copyId) const -> bool
               {
                  auto idxBuffer = toUnderlying(copyId);
                  if(!_unconsumedBuffers.at(idxBuffer).empty())
                    {
                       return false;
                    }
                  return _src->isFinished();
               }
             
             template<typename T>
               void InternalCopySourceConverter<T>::read(mods::utils::RWBuffer<T>* buf, CopyDestId copyId)
                 {
                    size_t read = 0;
                    auto idxBuffer = toUnderlying(copyId);
                    auto& out = *buf;
                    while(!_unconsumedBuffers.at(idxBuffer).empty() && read < out.size())
                      {
                         auto value = _unconsumedBuffers.at(idxBuffer).front();
                         out[read++] = value;
                         _unconsumedBuffers.at(idxBuffer).pop_front();
                      }
                    if(read < out.size())
                      {
                         auto remainingBuffer = buf->template slice<T>(read, out.size()-read);
                         _src->read(&remainingBuffer);
                         for(size_t i=0; i < out.size()-read; ++i)
                           {
                              _unconsumedBuffers.at(1-idxBuffer).push_back(remainingBuffer[i]);
                           }
                      }
                 }
             
             template<typename T>
               ChannelCopyConverterSlave<T>::ChannelCopyConverterSlave(typename InternalCopySourceConverter<T>::sptr src, CopyDestId copyId)
                 : _src(std::move(src)),
               _id(copyId)
                 {
                 }
             
             template<typename T>
               auto ChannelCopyConverterSlave<T>::buildSlave() const -> typename Converter<T>::ptr
               {
                  class make_unique_enabler : public ChannelCopyConverterSlave<T>
                    {
                     public:
                       explicit make_unique_enabler(const typename InternalCopySourceConverter<T>::sptr& src)
                         : ChannelCopyConverterSlave(src, CopyDestId::SLAVE)
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
             
             template<typename T>
               auto ChannelCopyConverterSlave<T>::isFinished() const -> bool
               {
                  return _src->isFinished(_id);
               }
             
             template<typename T>
               void ChannelCopyConverterSlave<T>::read(mods::utils::RWBuffer<T>* buf)
                 {
                    _src->read(buf, _id);
                 }
          } // namespace impl
        
        template<typename T>
          ChannelCopyConverter<T>::ChannelCopyConverter(typename Converter<T>::ptr src)
            : impl::ChannelCopyConverterSlave<T>(std::make_unique<impl::InternalCopySourceConverter<T>>(std::move(src)), impl::CopyDestId::MASTER),
          _copy(impl::ChannelCopyConverterSlave<T>::buildSlave())
            {
            }
        
        template<typename T>
          auto ChannelCopyConverter<T>::getCopy() -> typename Converter<T>::ptr
          {
             return std::move(_copy);
          }
        
        template class ChannelCopyConverter<s16>;
        template class ChannelCopyConverter<s32>;
        template class ChannelCopyConverter<double>;
     } // namespace converters
} // namespace mods
