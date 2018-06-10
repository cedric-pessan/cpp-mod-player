#ifndef _BUFFER_HPP_
#define _BUFFER_HPP_

#include "types.hpp"

#include <memory>
#include <iostream>

namespace mods
{
   template<typename T> class RBuffer;
   
   class Buffer
     {
      /*public:
        typedef std::shared_ptr<Buffer> sptr;
        typedef std::function<void()> callback;
        
        Buffer(u8* buf, callback freeCallback);
        ~Buffer();*/
        
      private:
        Buffer() = delete;
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        ~Buffer();
        
        /*u8* _buf;
        callback _freeResourceCallback;*/
        
      public:
        class Attorney
          {
           private:
             Attorney() = delete;
             Attorney(const Attorney&) = delete;
             Attorney& operator=(const Attorney&) = delete;
             ~Attorney() = delete;
             
             static u8* getBuffer()
               {
                  std::cout << "TODO: Buffer::Attorney::getBuffer()" << std::endl;
                  return nullptr;
               }
             
             template<typename T>
               friend class RBuffer;
          };
     };
}

#endif // _BUFFER_HPP_
