
#include "mods/utils/Buffer.hpp"

namespace mods
{
   namespace utils
     {
        Buffer::Buffer(u8* buf, size_t length, Deleter::ptr deleter)
          : _buf(buf),
          _length(length),
          _deleter(std::move(deleter))
            {
            }
     } // namespace utils
} // namespace mods
