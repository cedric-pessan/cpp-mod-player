#ifndef MODS_UTILS_RBUFFERBACKEND_HPP
#define MODS_UTILS_RBUFFERBACKEND_HPP

#include "types.hpp"

#include <iostream>
#include <memory>

namespace mods
{
   namespace utils
     {
        class RBufferBackend
          {
           public:
             using sptr = std::shared_ptr<RBufferBackend>;
             using value_type = const u8;
             
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
             
             RBufferBackend(const u8* buf, size_t length, Deleter::ptr deleter);
             ~RBufferBackend() = default;
             
             RBufferBackend() = delete;
             RBufferBackend(const RBufferBackend&) = delete;
             RBufferBackend(const RBufferBackend&&) = delete;
             RBufferBackend& operator=(const RBufferBackend&) = delete;
             RBufferBackend& operator=(const RBufferBackend&&) = delete;
             
           private:
             const u8* _buf;
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
                  static const u8* getBuffer(const RBufferBackend& buffer)
                    {
                       return buffer._buf;
                    }
                  
                  static size_t getLength(const RBufferBackend& buffer)
                    {
                       return buffer._length;
                    }
                  
                  template<typename T>
                    friend class RBuffer;
               };
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_RBUFFERBACKEND_HPP
