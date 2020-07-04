#ifndef MODS_UTILS_RBUFFER_HPP
#define MODS_UTILS_RBUFFER_HPP

#include "RBufferBackend.hpp"

#include <cassert>

namespace mods
{
   namespace utils
     {
        template<typename T>
          class RBuffer
          {
           public:
             using size_type = size_t;
             using const_reference = const T&;
             using backend_type = RBufferBackend;
             
             explicit RBuffer(RBufferBackend::sptr backend)
               : _backend(std::move(backend)),
               _buf(static_cast<const T*>(static_cast<const void*>(RBufferBackend::Attorney::getBuffer(*_backend)))),
               _len(RBufferBackend::Attorney::getLength(*_backend) / sizeof(T))
                 {
                 }
             
	     RBuffer() = delete;
             RBuffer(const RBuffer&) = default;
             RBuffer(RBuffer&&) noexcept = default;
             
             ~RBuffer() = default;
             
             auto operator=(const RBuffer&) -> RBuffer& = default;
             auto operator=(RBuffer&&) noexcept -> RBuffer& = default;
             
             auto operator->() const -> const T*
               {
                  return _buf;
               }
             
             template<typename T2>
               auto slice(size_t offset, size_t len) const -> RBuffer<T2>
               {
                  using TBuf = RBuffer<T2>;
                  return buildSlice<TBuf, T2, const T*>(_buf, offset, len);
               }
             
             auto size() const noexcept -> size_type
               {
                  return _len;
               }
             
             auto operator[](size_type pos) const -> const_reference
               {
                  assert(check(pos < _len, "out of bound RBuffer[]"));
                  return *(_buf + pos); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
               }
             
             class const_iterator
               {
                public:
                  using iterator_category = std::input_iterator_tag;
                  using self_type = const_iterator;
                  using value_type = T;
                  using reference = T&;
                  using pointer = T*;
                  using difference_type = int;
                  
                private:
                  const_iterator(const RBuffer<T>& rbuf, size_type pos)
                    : _pos(pos),
                    _rbuf(&rbuf)
                      {
                      }
                  friend class RBuffer; // only RBuffer can create these
                  
                public:
                  const_iterator(const_iterator&&) noexcept = default;
                  const_iterator(const const_iterator&) = default;
                  auto operator=(const_iterator&&) noexcept -> const_iterator& = default;
                  ~const_iterator() = default;
                  
                  const_iterator() = delete;
                  auto operator=(const const_iterator&) -> const_iterator& = delete;
                  
                  auto operator==(const const_iterator& obj) const -> bool
                    {
                       return _pos == obj._pos;
                    }
                  
                  auto operator!=(const const_iterator& obj) const -> bool
                    {
                       return !(*this == obj);
                    }
                  
                  auto operator<(const const_iterator& obj) const -> bool
                    {
                       return _pos < obj._pos;
                    }
                  
                  auto operator>=(const const_iterator& obj) const -> bool
                    {
                       return _pos >= obj._pos;
                    }
                  
                  auto operator*() const -> const_reference
                    {
                       return (*_rbuf)[_pos];
                    }
                  
                  auto operator++() -> const_iterator
                    {
                       ++_pos;
                       return *this;
                    }
                  
                  auto operator+=(ssize_t n) -> const_iterator
                    {
                       _pos += n;
                       return *this;
                    }
                  
                private:
                  size_type _pos;
                  const RBuffer<T>* _rbuf;
               };
             
             auto begin() const noexcept -> const_iterator
               {
                  return const_iterator(*this, 0);
               }
             
             auto end() const noexcept -> const_iterator
               {
                  return const_iterator(*this, size());
               }
             
           private:
             RBufferBackend::sptr _backend;
             const T* _buf;
             size_t _len;
             
           protected:
             auto check(bool condition, const std::string& description) const -> bool
               {
                  if(!condition)
                    {
                       throw std::out_of_range(description);
                    }
                  return true;
               }
             
             RBuffer(RBufferBackend::sptr backend, size_t offset, size_t len)
               : _backend(std::move(backend)),
               _buf(static_cast<const T*>(static_cast<const void*>(RBufferBackend::Attorney::getBuffer(*_backend) + offset))), // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
               _len(len)
                 {
                 }
             
             template<typename T2>
               friend class RBuffer;
             
             template<typename Buf, typename T2, typename InternalBuf>
               auto buildSlice(InternalBuf buf, size_t offset, size_t len) const -> Buf
               {
                  using BackendType = typename Buf::backend_type;
                  auto backend = std::static_pointer_cast<BackendType>(_backend);
                  size_t currentOffset = static_cast<const u8*>(static_cast<const void*>(buf)) - BackendType::Attorney::getBuffer(*backend);
                  check(offset * sizeof(T) + len * sizeof(T2) <= _len * sizeof(T), "invalid slice limits");
                  return Buf(backend, currentOffset + offset * sizeof(T), len);
               }
          };
     } // namespace utils
} // namespace mods
#endif // MODS_UTILS_RBUFFER_HPP