
#include "mods/utils/RBufferBackend.hpp"

namespace mods
{
   namespace utils
     {
        RBufferBackend::RBufferBackend(const u8* buf, size_t length, Deleter::ptr deleter)
          : _buf(buf),
          _length(length),
          _deleter(std::move(deleter))
            {
            }
     } // namespace utils
} // namespace mods
