#ifndef _RBUFFER_HPP_
#define _RBUFFER_HPP_

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
             typedef size_t size_type;
             typedef const T& const_reference;
             
             explicit RBuffer(const Buffer::sptr& backend)
               : _backend(backend),
               _rbuf(reinterpret_cast<T*>(Buffer::Attorney::getBuffer(*_backend))),
               _len(Buffer::Attorney::getLength(*_backend) / sizeof(T))
                 {
                 }
             
             RBuffer(RBuffer&&);
             
             ~RBuffer()
               {
               }
             
             T* operator->() const
               {
                  return _rbuf;
               }
             
             template<typename T2>
               RBuffer<T2> slice(size_t offset, size_t len) const
               {
                  size_t currentOffset = reinterpret_cast<u8*>(_rbuf) - reinterpret_cast<u8*>(Buffer::Attorney::getBuffer(*_backend));
                  check(offset * sizeof(T) + len * sizeof(T2) <= _len * sizeof(T), "invalid slice limits");
                  return RBuffer<T2>(_backend, currentOffset + offset * sizeof(T), len);
               }
             
             size_type size() const noexcept
               {
                  return _len;
               }
             
             const_reference operator[](size_type pos) const
               {
                  return *(_rbuf + pos);
               }
             
           private:
             RBuffer(const Buffer::sptr& backend, size_t offset, size_t len)
               : _backend(backend),
               _rbuf(reinterpret_cast<T*>(Buffer::Attorney::getBuffer(*_backend) + offset)),
               _len(len)
                 {
                 }
             RBuffer(const RBuffer&) = delete;
             RBuffer& operator=(const RBuffer&) = delete;
             
             template<typename T2>
               friend class RBuffer;
             
             void check(bool condition, const std::string& description) const
               {
                  if(!condition) throw std::out_of_range(description);
               }
             
             Buffer::sptr _backend;
             T* _rbuf;
             size_t _len;
          };
     }
}
#endif // _RBUFFER_HPP_