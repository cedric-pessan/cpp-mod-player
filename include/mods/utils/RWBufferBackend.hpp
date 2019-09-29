#ifndef MODS_UTILS_RWBUFFERBACKEND_HPP
#define MODS_UTILS_RWBUFFERBACKEND_HPP

#include "mods/utils/RBufferBackend.hpp"

namespace mods
{
   namespace utils
     {
        class RWBufferBackend : public RBufferBackend
          {
           public:
             using sptr = std::shared_ptr<RWBufferBackend>;
             using value_type = u8;
             
             using EmptyDeleter = RBufferBackend::EmptyDeleter;
             
             RWBufferBackend(u8* buf, size_t length, Deleter::ptr deleter);
             ~RWBufferBackend() = default;
             
             RWBufferBackend() = delete;
             RWBufferBackend(const RWBufferBackend&) = delete;
             RWBufferBackend(const RWBufferBackend&&) = delete;
             RWBufferBackend& operator=(const RWBufferBackend&) = delete;
             RWBufferBackend& operator=(const RWBufferBackend&&) = delete;
             
           private:
             u8* _buf;
             
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
                  static u8* getBuffer(const RWBufferBackend& buffer)
                    {
                       return buffer._buf;
                    }
                  
                  template<typename T>
                    friend class RWBuffer;
                  
                  template<typename T>
                    friend class RBuffer;
               };
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_RWBUFFERBACKEND_HPP
