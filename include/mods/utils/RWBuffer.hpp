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
             
             explicit RWBuffer(const RWBufferBackend::sptr& backend)
               : RBuffer<T>(backend),
               _buf(static_cast<T*>(static_cast<void*>(RWBufferBackend::Attorney::getBuffer(*backend))))
                 {
                 }
             
             RWBuffer() = delete;
             RWBuffer(const RWBuffer&) = default;
             RWBuffer(RWBuffer&&) noexcept = default;
             auto operator=(const RWBuffer&) -> RWBuffer& = delete;
             auto operator=(RWBuffer&&) noexcept -> RWBuffer& = default;
             ~RWBuffer() = default;
             
             template<typename T2>
               auto slice(size_t offset, size_t len) -> RWBuffer<T2>
                 {
                    using TBuf = RWBuffer<T2>;
                    return this->template buildSlice<TBuf, T2, T*>(_buf, offset, len);
                 }
             
             template<typename T2>
               auto readOnlySlice(size_t offset, size_t len) -> RBuffer<T2>
                 {
                    return RBuffer<T>::template slice<T2>(offset, len);
                 }
             
             auto operator[](size_type pos) -> reference
               {
                  assert(this->template check(pos < this->template size(), "out of bound RWBuffer"));
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
                  auto operator=(const iterator&) -> iterator& = delete;
                  auto operator=(iterator&&) -> iterator& = delete;
                  ~iterator() = default;
                  
                  auto operator==(const iterator& obj) const -> bool
                    {
                       return _pos == obj._pos;
                    }
                  
                  auto operator!=(const iterator& obj) const -> bool
                    {
                       return !(*this == obj);
                    }
                  
                  auto operator*() const -> reference
                    {
                       assert(check(_pos < _rwbuf.size(), "out of bound iterator dereferenced"));
                       return _rwbuf[_pos];
                    }
                  
                  auto operator++() -> iterator
                    {
                       ++_pos;
                       return *this;
                    }
                  
                private:
                  auto check(bool condition, const std::string& description) const -> bool
                    {
                       if(!condition)
                         {
                            throw std::out_of_range(description);
                         }
                       return true;
                    }
                  
                  size_type _pos;
                  RWBuffer<T>& _rwbuf;
               };
             
             auto begin() noexcept -> iterator
               {
                  return iterator(*this, 0);
               }
             
             auto end() noexcept -> iterator
               {
                  return iterator(*this, RBuffer<T>::size());
               }
             
           private:
             RWBuffer(const RWBufferBackend::sptr& backend, size_t offset, size_t len)
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
