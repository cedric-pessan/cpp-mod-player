#ifndef MODS_UTILS_RBUFFERBACKEND_HPP
#define MODS_UTILS_RBUFFERBACKEND_HPP

#include "types.hpp"

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
                  Deleter(Deleter&&) = delete;
                  auto operator=(const Deleter&) -> Deleter& = delete;
                  auto operator=(Deleter&&) -> Deleter& = delete;
               };
             
             class EmptyDeleter : public Deleter
               {
                public:
                  EmptyDeleter() = default;
                  ~EmptyDeleter() override = default;
                  
                  EmptyDeleter(const EmptyDeleter&) = delete;
                  EmptyDeleter(EmptyDeleter&&) = delete;
                  auto operator=(const EmptyDeleter&) -> EmptyDeleter& = delete;
                  auto operator=(EmptyDeleter&&) -> EmptyDeleter& = delete;
               };
             
             RBufferBackend(const u8* buf, size_t length, Deleter::ptr deleter);
             ~RBufferBackend() = default;
             
             RBufferBackend() = delete;
             RBufferBackend(const RBufferBackend&) = delete;
             RBufferBackend(RBufferBackend&&) = delete;
             auto operator=(const RBufferBackend&) -> RBufferBackend& = delete;
             auto operator=(RBufferBackend&&) -> RBufferBackend& = delete;
             
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
                  Attorney(Attorney&&) = delete;
                  auto operator=(const Attorney&) -> Attorney& = delete;
                  auto operator=(Attorney&&) -> Attorney& = delete;
                  ~Attorney() = delete;
                  
                private:
                  static auto getBuffer(const RBufferBackend& buffer) -> const u8*
                    {
                       return buffer._buf;
                    }
                  
                  static auto getLength(const RBufferBackend& buffer) -> size_t
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
