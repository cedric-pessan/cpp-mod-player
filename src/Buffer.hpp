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
      public:
        typedef std::shared_ptr<Buffer> sptr;
        class Deleter
          {
           public:
             typedef std::unique_ptr<Deleter> ptr;
             
             Deleter();
             virtual ~Deleter();
             
           private:
             Deleter(const Deleter&) = delete;
             Deleter& operator=(const Deleter&) = delete;
          };
        
        Buffer(u8* buf, size_t length, Deleter::ptr deleter);
        ~Buffer();
        
      private:
        Buffer() = delete;
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        
        u8* _buf;
        size_t _length;
        Deleter::ptr _deleter;
        
      public:
        class Attorney
          {
           private:
             Attorney() = delete;
             Attorney(const Attorney&) = delete;
             Attorney& operator=(const Attorney&) = delete;
             ~Attorney() = delete;
             
             static u8* getBuffer(Buffer& buffer)
               {
                  return buffer._buf;
               }
             
             static size_t getLength(Buffer& buffer)
               {
                  return buffer._length;
               }
             
             template<typename T>
               friend class RBuffer;
          };
     };
}

#endif // _BUFFER_HPP_
