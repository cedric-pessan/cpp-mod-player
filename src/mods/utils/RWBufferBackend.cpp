
#include "mods/utils/RBufferBackend.hpp"
#include "mods/utils/RWBufferBackend.hpp"
#include "mods/utils/types.hpp"

#include <cstddef>
#include <utility>

namespace mods
{
   namespace utils
     {
        template<typename T>
          RWBufferBackend::RWBufferBackend(T* buf, size_t length, Deleter::ptr deleter)
            : RBufferBackend(buf, length, std::move(deleter)),
          _buf(reinterpret_cast<u8*>(buf)) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
            {
            }
        
        template RWBufferBackend::RWBufferBackend(u8* buf, size_t length, Deleter::ptr deleter);
        template RWBufferBackend::RWBufferBackend(s16* buf, size_t length, Deleter::ptr delter);
     } // namespace utils
} // namespace mods
