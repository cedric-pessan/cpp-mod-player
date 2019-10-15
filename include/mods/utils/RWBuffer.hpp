#ifndef MODS_UTILS_RWBUFFER_HPP
#define MODS_UTILS_RWBUFFER_HPP

#include "mods/utils/RBuffer.hpp"
#include "mods/utils/RWBufferBackend.hpp"

namespace mods
{
   namespace utils
     {
        template<typename T>
          class RWBuffer : public RBuffer<T>
          {
           public:
             using size_type = size_t;
             using reference = T&;
             using backend_type = RWBufferBackend;
             
             explicit RWBuffer(RWBufferBackend::sptr backend)
               : RBuffer<T>(backend),
               _buf(static_cast<T*>(static_cast<void*>(RWBufferBackend::Attorney::getBuffer(*backend))))
                 {
                 }
             
             RWBuffer() = delete;
             RWBuffer(const RWBuffer&) = default;
             RWBuffer(RWBuffer&&) noexcept = default;
             RWBuffer& operator=(const RWBuffer&) = default;
             RWBuffer& operator=(RWBuffer&&) noexcept = default;
             ~RWBuffer() = default;
             
             template<typename T2>
               const RWBuffer<T2> slice(size_t offset, size_t len)
                 {
                    using TBuf = RWBuffer<T2>;
                    return this->template buildSlice<TBuf, T2, T*>(_buf, offset, len);
                 }
             
             reference operator[](size_type pos)
               {
                  return *(_buf + pos); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
               }
             
             class iterator
               {
                private:
                  iterator(RWBuffer<T>& rwbuf, size_type pos)
                    : _pos(pos),
                    _rwbuf(rwbuf)
                      {
                      }
                  friend class RWBuffer; // only RWBuffer can create these
                  
                public:
                  iterator() = delete;
                  iterator(const iterator&) = default;
                  iterator(iterator&&) noexcept = default;
                  iterator& operator=(const iterator&) = delete;
                  iterator& operator=(const iterator&&) = delete;
                  ~iterator() = default;
                  
                  bool operator==(const iterator& obj) const
                    {
                       return _pos == obj._pos;
                    }
                  
                  bool operator!=(const iterator& obj) const
                    {
                       return !(*this == obj);
                    }
                  
                  reference operator*() const
                    {
                       return _rwbuf[_pos];
                    }
                  
                  iterator operator++()
                    {
                       ++_pos;
                       return *this;
                    }
                  
                private:
                  size_type _pos;
                  RWBuffer<T>& _rwbuf;
               };
             
             iterator begin() noexcept
               {
                  return iterator(*this, 0);
               }
             
             iterator end() noexcept
               {
                  return iterator(*this, this->template size());
               }
             
           private:
             RWBuffer(RWBufferBackend::sptr backend, size_t offset, size_t len)
               : RBuffer<T>(backend, offset, len),
               _buf(static_cast<T*>(static_cast<void*>(RWBufferBackend::Attorney::getBuffer(*backend) + offset))) // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                 {
                 }
             
             template<typename T2>
               friend class RBuffer;
             
             T* _buf;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_RWBUFFER_HPP
