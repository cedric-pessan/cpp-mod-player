
#include "mods/utils/RWBufferBackend.hpp"

namespace mods
{
   namespace utils
     {
        RWBufferBackend::RWBufferBackend(u8* buf, size_t length, Deleter::ptr deleter)
          : RBufferBackend(buf, length, std::move(deleter)),
          _buf(buf)
            {
            }
     } // namespace utils
} // namespace mods
