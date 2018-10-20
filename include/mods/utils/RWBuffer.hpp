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
             RWBuffer(const RWBuffer&) = default;
             RWBuffer(RWBuffer&&) = default;
             RWBuffer& operator=(const RWBuffer&) = delete;
             RWBuffer& operator=(RWBuffer&&) = default;
             ~RWBuffer() = default;
             
             template<typename T2>
               const RWBuffer<T2> slice(size_t offset, size_t len)
                 {
                    using TBuf = RWBuffer<T2>;
                    return this->template buildSlice<TBuf, T2>(offset, len);
                 }
             
             reference operator[](size_type pos)
               {
                  return *(RBuffer<T>::_buf + pos);
               }
             
             class iterator
               {
                private:
                  iterator(RWBuffer<T>& rwbuf, size_type pos)
                    : _pos(pos),
                    _rwbuf(rwbuf)
                      {
                      }
                  friend class RWBuffer; // only RWBuffer can create these
                  
                public:
                  iterator() = delete;
                  iterator(const iterator&) = default;
                  iterator(iterator&&) = default;
                  iterator& operator=(const iterator&) = delete;
                  iterator& operator=(const iterator&&) = delete;
                  ~iterator() = default;
                  
                  bool operator==(const iterator& obj) const
                    {
                       return _pos == obj._pos;
                    }
                  
                  bool operator!=(const iterator& obj) const
                    {
                       return !(*this == obj);
                    }
                  
                  reference operator*() const
                    {
                       return _rwbuf[_pos];
                    }
                  
                  iterator operator++()
                    {
                       ++_pos;
                       return *this;
                    }
                  
                private:
                  size_type _pos;
                  RWBuffer<T>& _rwbuf;
               };
             
             iterator begin() noexcept
               {
                  return iterator(*this, 0);
               }
             
             iterator end() noexcept
               {
                  return iterator(*this, this->template size());
               }
             
           private:
             RWBuffer(BufferBackend::sptr backend, size_t offset, size_t len)
               : RBuffer<T>(std::move(backend), offset, len)
                 {
                 }
             
             template<typename T2>
               friend class RBuffer;
          };
     }
}

#endif // MODS_UTILS_RWBUFFER_HPP
