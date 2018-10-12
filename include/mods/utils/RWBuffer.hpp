#ifndef MODS_UTILS_RWBUFFER_HPP
#define MODS_UTILS_RWBUFFER_HPP

#include "mods/utils/RBuffer.hpp"

namespace mods
{
   namespace utils
     {
        template<typename T>
          class RWBuffer : public RBuffer<T>
          {
           public:
             using size_type = size_t;
             using reference = T&;
             
             explicit RWBuffer(BufferBackend::sptr backend)
               : RBuffer<T>(backend)
                 {
                 }
             
             RWBuffer() = delete;
             RWBuffer(const RWBuffer&) = delete;
             RWBuffer(const RWBuffer&&) = delete;
             RWBuffer& operator=(const RWBuffer&) = delete;
             RWBuffer& operator=(const RWBuffer&&) = delete;
             ~RWBuffer() = default;
             
             reference operator[](size_type pos)
               {
                  return *(RBuffer<T>::_buf + pos);
               }
          };
     }
}

#endif // MODS_UTILS_RWBUFFER_HPP
