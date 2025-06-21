#ifndef MODS_UTILS_PACKEDARRAY_HPP
#define MODS_UTILS_PACKEDARRAY_HPP

#include <cassert>
#include <cstddef>

namespace mods
{
   namespace utils
     {
#pragma pack(push,1)
        template<typename T, int SIZE>
          class PackedArray
          {
           public:
             using value_type = T;
             
             PackedArray() = delete;
             PackedArray(const PackedArray&) = delete;
             PackedArray(PackedArray&&) = delete;
             auto operator=(const PackedArray&) -> PackedArray& = delete;
             auto operator=(PackedArray&&) -> PackedArray& = delete;
             ~PackedArray() = delete;
             
             auto data() const -> const T*
               {
                  return &_array[0];
               }
             
             auto operator[](std::size_t index) const -> const T&
               {
                  assert(index >= 0 && index < SIZE);
                  return _array[index]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
               }
             
             class Iterator
               {
                public:
                  using value_type = PackedArray::value_type;
                  using const_reference = const value_type&;
                  
                  Iterator(const PackedArray& packedArray, size_t pos) :
                    _packedArray(&packedArray),
                    _pos(pos)
                      {
                      }
                  
                  Iterator() = delete;
                  Iterator(const Iterator&) = delete;
                  Iterator(Iterator&&) noexcept = default;
                  auto operator=(const Iterator&) -> Iterator& = delete;
                  auto operator=(Iterator&&) -> Iterator& = delete;
                  ~Iterator() = default;
                  
                  auto operator!=(const Iterator& rightIt) -> bool
                    {
                       return _pos != rightIt._pos;
                    }
                  
                  auto operator++() -> Iterator&
                    {
                       ++_pos;
                       return *this;
                    }
                  
                  auto operator*() const -> const_reference
                    {
                       return (*_packedArray)[_pos];
                    }
                  
                private:
                  const PackedArray<T, SIZE>* _packedArray;
                  size_t _pos;
               };
             
             auto begin() const -> Iterator
               {
                  return Iterator(*this, 0);
               }
             
             auto end() const -> Iterator
               {
                  return Iterator(*this, SIZE);
               }
             
           private:
             T _array[SIZE]; // NOLINT(hicpp-avoid-c-arrays,modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
          };
#pragma pack(pop)
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_PACKEDARRAY_HPP
