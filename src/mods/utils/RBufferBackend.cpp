
#include "mods/utils/RBufferBackend.hpp"
#include "mods/utils/types.hpp"

#include <cstddef>
#include <utility>

namespace mods
{
   namespace utils
     {
        RBufferBackend::RBufferBackend(Deleter::ptr deleter)
          : _buf(nullptr),
          _length(0),
          _deleter(std::move(deleter))
            {
            }
        
        template<typename T>
          RBufferBackend::RBufferBackend(const T* buf, size_t length, Deleter::ptr deleter)
            : _buf(reinterpret_cast<const u8*>(buf)), // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
          _length(length * sizeof(T)),
          _deleter(std::move(deleter))
            {
            }
        
        template RBufferBackend::RBufferBackend(const double* buf, size_t length, Deleter::ptr deleter);
        template RBufferBackend::RBufferBackend(char* const * buf, size_t length, Deleter::ptr deleter);
        template RBufferBackend::RBufferBackend(const u8* buf, size_t length, Deleter::ptr deleter);
        template RBufferBackend::RBufferBackend(const s16* buf, size_t length, Deleter::ptr deleter);
        template RBufferBackend::RBufferBackend(const char* buf, size_t length, Deleter::ptr deleter);
     } // namespace utils
} // namespace mods
