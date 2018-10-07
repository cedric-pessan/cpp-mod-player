#ifndef MODS_UTILS_RBUFFER_HPP
#define MODS_UTILS_RBUFFER_HPP

#include "Buffer.hpp"

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
             
             explicit RBuffer(Buffer::sptr backend)
               : _backend(std::move(backend)),
               _rbuf(static_cast<T*>(static_cast<void*>(Buffer::Attorney::getBuffer(*_backend)))),
               _len(Buffer::Attorney::getLength(*_backend) / sizeof(T))
                 {
                 }
             
             RBuffer(const RBuffer&) = default;
             RBuffer(RBuffer&&) noexcept = default;
             
             ~RBuffer() = default;
             
             RBuffer& operator=(const RBuffer&) = delete;
             RBuffer& operator=(RBuffer&&) noexcept = default;
             
             T* operator->() const
               {
                  return _rbuf;
               }
             
             template<typename T2>
               RBuffer<T2> slice(size_t offset, size_t len) const
               {
                  size_t currentOffset = static_cast<u8*>(_rbuf) - static_cast<u8*>(Buffer::Attorney::getBuffer(*_backend));
                  check(offset * sizeof(T) + len * sizeof(T2) <= _len * sizeof(T), "invalid slice limits");
                  return RBuffer<T2>(_backend, currentOffset + offset * sizeof(T), len);
               }
             
             size_type size() const noexcept
               {
                  return _len;
               }
             
             const_reference operator[](size_type pos) const
               {
                  return *(_rbuf + pos); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
               }
             
             class const_iterator
               {
                private:
                  const_iterator(size_type pos)
                    : _pos(pos)
                      {
                      }
                  friend class RBuffer; // only RBuffer can create these
                  
                public:
                  const_iterator(const_iterator&&) = default;
                  ~const_iterator() = default;
                  
                  const_iterator() = delete;
                  const_iterator(const const_iterator&) = delete;
                  const_iterator& operator=(const const_iterator&) = delete;
                  const_iterator& operator=(const const_iterator&&) = delete;
                  
                  bool operator==(const const_iterator& obj) const
                    {
                       return _pos == obj._pos;
                    }
                  
                private:
                  size_type _pos;
               };
             
             const_iterator begin() const noexcept
               {
                  return const_iterator(0);
               }
             
             const_iterator end() const noexcept
               {
                  return const_iterator(size());
               }
             
           private:
             RBuffer(Buffer::sptr backend, size_t offset, size_t len)
               : _backend(std::move(backend)),
               _rbuf(static_cast<T*>(static_cast<void*>(Buffer::Attorney::getBuffer(*_backend) + offset))), // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
               _len(len)
                 {
                 }
             
             template<typename T2>
               friend class RBuffer;
             
             void check(bool condition, const std::string& description) const
               {
                  if(!condition)
                    {
                       throw std::out_of_range(description);
                    }
               }
             
             Buffer::sptr _backend;
             T* _rbuf;
             size_t _len;
          };
     } // namespace utils
} // namespace mods
#endif // MODS_UTILS_RBUFFER_HPP