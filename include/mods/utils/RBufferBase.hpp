#ifndef MODS_UTILS_RBUFFERBASE_HPP
#define MODS_UTILS_RBUFFERBASE_HPP

namespace mods
{
   namespace utils
     {
        class RBufferBase
          {
           public:
             enum Offset : size_t;
             enum Length : size_t;
             
	     RBufferBase() = default;
             RBufferBase(const RBufferBase&) = default;
             RBufferBase(RBufferBase&&) noexcept = default;
             
             ~RBufferBase() = default;
             
             auto operator=(const RBufferBase&) -> RBufferBase& = default;
             auto operator=(RBufferBase&&) noexcept -> RBufferBase& = default;
          };
     } // namespace utils
} // namespace mods
#endif // MODS_UTILS_RBUFFERBASE_HPP