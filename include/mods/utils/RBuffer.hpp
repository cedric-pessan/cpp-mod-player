#ifndef MODS_UTILS_RBUFFER_HPP
#define MODS_UTILS_RBUFFER_HPP

#include "BufferBackend.hpp"

#include <iostream>

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
             
             explicit RBuffer(BufferBackend::sptr backend)
               : _backend(std::move(backend)),
               _buf(static_cast<T*>(static_cast<void*>(BufferBackend::Attorney::getBuffer(*_backend)))),
               _len(BufferBackend::Attorney::getLength(*_backend) / sizeof(T))
                 {
                 }
             
             RBuffer(const RBuffer&) = default;
             RBuffer(RBuffer&&) noexcept = default;
             
             ~RBuffer() = default;
             
             RBuffer& operator=(const RBuffer&) = delete;
             RBuffer& operator=(RBuffer&&) noexcept = default;
             
             T* operator->() const
               {
                  return _buf;
               }
             
             template<typename T2>
               const RBuffer<T2> slice(size_t offset, size_t len) const
               {
                  using TBuf = RBuffer<T2>;
                  return buildSlice<TBuf, T2>(offset, len);
               }
             
             size_type size() const noexcept
               {
                  return _len;
               }
             
             const_reference operator[](size_type pos) const
               {
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
                  const_iterator& operator=(const_iterator&&) = default;
                  ~const_iterator() = default;
                  
                  const_iterator() = delete;
                  const_iterator& operator=(const const_iterator&) = delete;
                  
                  bool operator==(const const_iterator& obj) const
                    {
                       return _pos == obj._pos;
                    }
                  
                  bool operator!=(const const_iterator& obj) const
                    {
                       return !(*this == obj);
                    }
                  
                  bool operator<(const const_iterator& obj) const
                    {
                       return _pos < obj._pos;
                    }
                  
                  bool operator>=(const const_iterator& obj) const
                    {
                       return _pos >= obj._pos;
                    }
                  
                  const_reference operator*() const
                    {
                       return (*_rbuf)[_pos];
                    }
                  
                  const_iterator operator++()
                    {
                       ++_pos;
                       return *this;
                    }
                  
                  const_iterator operator+=(int n)
                    {
                       _pos += n;
                       return *this;
                    }
                  
                private:
                  size_type _pos;
                  const RBuffer<T>* _rbuf;
               };
             
             const_iterator begin() const noexcept
               {
                  return const_iterator(*this, 0);
               }
             
             const_iterator end() const noexcept
               {
                  return const_iterator(*this, size());
               }
             
           private:
             void check(bool condition, const std::string& description) const
               {
                  if(!condition)
                    {
                       throw std::out_of_range(description);
                    }
               }
             
             BufferBackend::sptr _backend;
             
           protected:
             RBuffer(BufferBackend::sptr backend, size_t offset, size_t len)
               : _backend(std::move(backend)),
               _buf(static_cast<T*>(static_cast<void*>(BufferBackend::Attorney::getBuffer(*_backend) + offset))), // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
               _len(len)
                 {
                 }
             
             template<typename T2>
               friend class RBuffer;
             
             template<typename Buf, typename T2>
               const Buf buildSlice(size_t offset, size_t len) const
               {
                  size_t currentOffset = static_cast<u8*>(_buf) - static_cast<u8*>(BufferBackend::Attorney::getBuffer(*_backend));
                  check(offset * sizeof(T) + len * sizeof(T2) <= _len * sizeof(T), "invalid slice limits");
                  return Buf(_backend, currentOffset + offset * sizeof(T), len);
               }
             
             T* _buf;
             size_t _len;
          };
     } // namespace utils
} // namespace mods
#endif // MODS_UTILS_RBUFFER_HPP