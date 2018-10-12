#ifndef MODS_UTILS_BUFFERBACKEND_HPP
#define MODS_UTILS_BUFFERBACKEND_HPP

#include "types.hpp"

#include <iostream>
#include <memory>

namespace mods
{
   namespace utils
     {
        template<typename T> class RBuffer;
        
        class BufferBackend
          {
           public:
             using sptr = std::shared_ptr<BufferBackend>;
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
             
             class EmptyDeleter : public Deleter
               {
                public:
                  EmptyDeleter() = default;
                  ~EmptyDeleter() override = default;
                  
                  EmptyDeleter(const EmptyDeleter&) = delete;
                  EmptyDeleter(const EmptyDeleter&&) = delete;
                  EmptyDeleter& operator=(const EmptyDeleter&) = delete;
                  EmptyDeleter& operator=(const EmptyDeleter&&) = delete;
               };
             
             BufferBackend(u8* buf, size_t length, Deleter::ptr deleter);
             ~BufferBackend() = default;
             
             BufferBackend() = delete;
             BufferBackend(const BufferBackend&) = delete;
             BufferBackend(const BufferBackend&&) = delete;
             BufferBackend& operator=(const BufferBackend&) = delete;
             BufferBackend& operator=(const BufferBackend&&) = delete;
             
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
                  static u8* getBuffer(const BufferBackend& buffer)
                    {
                       return buffer._buf;
                    }
                  
                  static size_t getLength(const BufferBackend& buffer)
                    {
                       return buffer._length;
                    }
                  
                  template<typename T>
                    friend class RBuffer;
               };
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_BUFFERBACKEND_HPP
