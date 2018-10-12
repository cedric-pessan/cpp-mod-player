
#include "mods/utils/BufferBackend.hpp"

namespace mods
{
   namespace utils
     {
        BufferBackend::BufferBackend(u8* buf, size_t length, Deleter::ptr deleter)
          : _buf(buf),
          _length(length),
          _deleter(std::move(deleter))
            {
            }
     } // namespace utils
} // namespace mods
