#ifndef MODS_UTILS_BUFFER_HPP
#define MODS_UTILS_BUFFER_HPP

#include "types.hpp"

#include <iostream>
#include <memory>

namespace mods
{
   namespace utils
     {
        template<typename T> class RBuffer;
        
        class Buffer
          {
           public:
             using sptr = std::shared_ptr<Buffer>;
             class Deleter
               {
                public:
                  using ptr = std::unique_ptr<Deleter>;
                  
                  Deleter() = default;
                  virtual ~Deleter() = default;
                  
                  Deleter(const Deleter&) = delete;
                  Deleter(const Deleter&&) = delete;
                  Deleter& operator=(const Deleter&) = delete;
                  Deleter& operator=(const Deleter&&) = delete;
               };
             
             Buffer(u8* buf, size_t length, Deleter::ptr deleter);
             ~Buffer() = default;
             
             Buffer() = delete;
             Buffer(const Buffer&) = delete;
             Buffer(const Buffer&&) = delete;
             Buffer& operator=(const Buffer&) = delete;
             Buffer& operator=(const Buffer&&) = delete;
             
           private:
             u8* _buf;
             size_t _length;
             Deleter::ptr _deleter;
             
           public:
             class Attorney
               {
                public:
                  Attorney() = delete;
                  Attorney(const Attorney&) = delete;
                  Attorney(const Attorney&&) = delete;
                  Attorney& operator=(const Attorney&) = delete;
                  Attorney& operator=(const Attorney&&) = delete;
                  ~Attorney() = delete;
                  
                private:
                  static u8* getBuffer(const Buffer& buffer)
                    {
                       return buffer._buf;
                    }
                  
                  static size_t getLength(const Buffer& buffer)
                    {
                       return buffer._length;
                    }
                  
                  template<typename T>
                    friend class RBuffer;
               };
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_BUFFER_HPP
