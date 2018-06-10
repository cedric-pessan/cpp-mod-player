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
        RBuffer()
          {
             /*_backend = std::make_shared<Buffer>();*/
             _rbuf = reinterpret_cast<T*>(Buffer::Attorney::getBuffer());
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
          RBuffer<T2> slice() 
            {
               std::cout << "TODO: RBuffer<T>::slice()" << std::endl;
               return RBuffer<T2>();
            }
        
      private:
        /*RBuffer(Buffer::sptr backend, size_t offset)
          : _backend(backend)
          {
             _rbuf = _buf + offset;
          }*/
        RBuffer(const RBuffer&) = delete;
        RBuffer& operator=(const RBuffer&) = delete;
        
        /*Buffer::sptr _backend;*/
        T* _rbuf;
     };
}

#endif // _RBUFFER_HPP_