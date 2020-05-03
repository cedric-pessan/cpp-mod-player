#ifndef MODS_UTILS_PACKEDARRAY_HPP
#define MODS_UTILS_PACKEDARRAY_HPP

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
                  return _array[index];
               }
             
           private:
             T _array[SIZE]; // NOLINT(hicpp-avoid-c-arrays,modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
          };
#pragma pack(pop)
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_PACKEDARRAY_HPP
