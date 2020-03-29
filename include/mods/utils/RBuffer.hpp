#ifndef MODS_UTILS_RBUFFER_HPP
#define MODS_UTILS_RBUFFER_HPP

#include "RBufferBackend.hpp"

#include <cassert>

namespace mods
{
   namespace utils
     {
        template<typename T>
          class RBuffer
          {
           public:
             using size_type = size_t;
             using const_reference = const T&;
             using backend_type = RBufferBackend;
             
             explicit RBuffer(RBufferBackend::sptr backend)
               : _backend(std::move(backend)),
               _buf(static_cast<const T*>(static_cast<const void*>(RBufferBackend::Attorney::getBuffer(*_backend)))),
               _len(RBufferBackend::Attorney::getLength(*_backend) / sizeof(T))
                 {
                 }
             
	     RBuffer() = delete;
             RBuffer(const RBuffer&) = default;
             RBuffer(RBuffer&&) noexcept = default;
             
             ~RBuffer() = default;
             
             RBuffer& operator=(const RBuffer&) = default;
             RBuffer& operator=(RBuffer&&) noexcept = default;
             
             const T* operator->() const
               {
                  return _buf;
               }
             
             template<typename T2>
               const RBuffer<T2> slice(size_t offset, size_t len) const
               {
                  using TBuf = RBuffer<T2>;
                  return buildSlice<TBuf, T2, const T*>(_buf, offset, len);
               }
             
             size_type size() const noexcept
               {
                  return _len;
               }
             
             const_reference operator[](size_type pos) const
               {
                  assert(check(pos < _len, "out of bound RBuffer[]"));
                  return *(_buf + pos); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
               }
             
             class const_iterator
               {
                public:
                  using iterator_category = std::input_iterator_tag;
                  using self_type = const_iterator;
                  using value_type = T;
                  using reference = T&;
                  using pointer = T*;
                  using difference_type = int;
                  
                private:
                  const_iterator(const RBuffer<T>& rbuf, size_type pos)
                    : _pos(pos),
                    _rbuf(&rbuf)
                      {
                      }
                  friend class RBuffer; // only RBuffer can create these
                  
                public:
                  const_iterator(const_iterator&&) noexcept = default;
                  const_iterator(const const_iterator&) = default;
                  const_iterator& operator=(const_iterator&&) noexcept = default;
                  ~const_iterator() = default;
                  
                  const_iterator() = delete;
                  const_iterator& operator=(const const_iterator&) = delete;
                  
                  bool operator==(const const_iterator& obj) const
                    {
                       return _pos == obj._pos;
                    }
                  
                  bool operator!=(const const_iterator& obj) const
                    {
                       return !(*this == obj);
                    }
                  
                  bool operator<(const const_iterator& obj) const
                    {
                       return _pos < obj._pos;
                    }
                  
                  bool operator>=(const const_iterator& obj) const
                    {
                       return _pos >= obj._pos;
                    }
                  
                  const_reference operator*() const
                    {
                       return (*_rbuf)[_pos];
                    }
                  
                  const_iterator operator++()
                    {
                       ++_pos;
                       return *this;
                    }
                  
                  const_iterator operator+=(int n)
                    {
                       _pos += n;
                       return *this;
                    }
                  
                private:
                  size_type _pos;
                  const RBuffer<T>* _rbuf;
               };
             
             const_iterator begin() const noexcept
               {
                  return const_iterator(*this, 0);
               }
             
             const_iterator end() const noexcept
               {
                  return const_iterator(*this, size());
               }
             
           private:
             RBufferBackend::sptr _backend;
             const T* _buf;
             
           protected:
             bool check(bool condition, const std::string& description) const
               {
                  if(!condition)
                    {
                       throw std::out_of_range(description);
                    }
                  return true;
               }
             
             RBuffer(RBufferBackend::sptr backend, size_t offset, size_t len)
               : _backend(std::move(backend)),
               _buf(static_cast<const T*>(static_cast<const void*>(RBufferBackend::Attorney::getBuffer(*_backend) + offset))), // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
               _len(len)
                 {
                 }
             
             template<typename T2>
               friend class RBuffer;
             
             template<typename Buf, typename T2, typename InternalBuf>
               const Buf buildSlice(InternalBuf buf, size_t offset, size_t len) const
               {
                  using BackendType = typename Buf::backend_type;
                  using ValueType = typename BackendType::value_type;
                  auto backend = std::static_pointer_cast<BackendType>(_backend);
                  size_t currentOffset = static_cast<ValueType*>(buf) - static_cast<ValueType*>(BackendType::Attorney::getBuffer(*backend));
                  check(offset * sizeof(T) + len * sizeof(T2) <= _len * sizeof(T), "invalid slice limits");
                  return Buf(backend, currentOffset + offset * sizeof(T), len);
               }
             
             size_t _len;
          };
     } // namespace utils
} // namespace mods
#endif // MODS_UTILS_RBUFFER_HPP