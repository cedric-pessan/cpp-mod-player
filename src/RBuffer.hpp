#ifndef _RBUFFER_HPP_
#define _RBUFFER_HPP_

#include "Buffer.hpp"

#include <iostream>

namespace mods
{
   template<typename T>
     class RBuffer
     {
      public:
        explicit RBuffer(const Buffer::sptr& backend)
          : _backend(backend),
          _rbuf(reinterpret_cast<T*>(Buffer::Attorney::getBuffer(*_backend))),
          _len(Buffer::Attorney::getLength(*_backend))
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
          RBuffer<T2> slice(size_t offset, size_t len) 
            {
               check(offset + len * sizeof(T2) <= _len, "invalid slice limits");
               return RBuffer<T2>(_backend, offset, len);
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
          template<typename T3>
          friend RBuffer<T3> RBuffer<T2>::slice(size_t offset, size_t len);
        
        void check(bool condition, const std::string& description)
          {
             if(!condition) throw std::out_of_range(description);
          }
        
        Buffer::sptr _backend;
        T* _rbuf;
        size_t _len;
     };
}

#endif // _RBUFFER_HPP_