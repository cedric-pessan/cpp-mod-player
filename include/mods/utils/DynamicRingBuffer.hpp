#ifndef MODS_UTILS_DYNAMICRINGBUFFER_HPP
#define MODS_UTILS_DYNAMICRINGBUFFER_HPP

#include <stdexcept>
#include <type_traits>
#include <vector>

namespace mods
{
   namespace utils
     {
        /*
         * the goal is to have a minimum api similar to deque but without chunk size dependent on compiler
         * implementation. This assumes that size remains nearly constant over time but can't be known at compile time.
         * It only supports arithmetic types (int, float, etc..).
         */
        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value, int>::type = 0>
          class DynamicRingBuffer
          {
           public:
             using const_reference = const T&;
             using value_type = T;
             using size_type = std::size_t;
             
             DynamicRingBuffer() = default;
             DynamicRingBuffer(const DynamicRingBuffer&) = delete;
             DynamicRingBuffer(DynamicRingBuffer&&) = delete;
             auto operator=(const DynamicRingBuffer&) = delete;
             auto operator=(DynamicRingBuffer&&) = delete;
             ~DynamicRingBuffer() = default;
             
             auto empty() const noexcept -> bool
               {
                  return _begin == _end;
               }
             
             auto front() const -> const_reference
               {
                  if(_begin == _end)
                    {
                       throw std::runtime_error("empty dynamic ring buffer");
                    }
                  return _backend[_begin];
               }
             
             void pop_front()
               {
                  ++_begin;
                  if(_begin == _backend.size())
                    {
                       _begin = 0;
                    }
               }
             
             void push_back(const value_type& val)
               {
                  auto nextEnd = _end + 1;
                  if(nextEnd >= _backend.size())
                    {
                       nextEnd = 0;
                       if(_backend.empty())
                         {
                            increaseBackendSize();
                            nextEnd = _end + 1;
                            if(nextEnd == _backend.size())
                              {
                                 nextEnd = 0;
                              }
                         }
                    }
                  if(nextEnd == _begin)
                    {
                       increaseBackendSize();
                       nextEnd = _end + 1;
                       if(nextEnd == _backend.size())
                         {
                             nextEnd = 0;
                         }
                    }
                  _backend[_end] = val;
                  _end = nextEnd;
               }
             
           private:
             void increaseBackendSize()
               {
                  size_type copySize = 0;
                  if(_end < _begin)
                    {
                       copySize = _end;
                    }
                  
                  auto oldSize = _backend.size();
                  auto newSize = oldSize * 2 + 1;
                  _backend.resize(newSize);
                  
                  if(_end < _begin)
                    {
                       std::copy(_backend.begin(), _backend.begin() + copySize, _backend.begin() + oldSize);
                       _end = oldSize + copySize;
                    }
               }
             
             size_type _begin = 0;
             size_type _end = 0;
             
             std::vector<T> _backend;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_DYNAMICRINGBUFFER_HPP
