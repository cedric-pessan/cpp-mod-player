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
             
             RBuffer(RBuffer&&) noexcept = default;
             
             ~RBuffer() = default;
             
             RBuffer(const RBuffer&) = delete;
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