
#ifndef MODS_UTILS_AT_HPP
#define MODS_UTILS_AT_HPP

#include <cassert>
#include <cstdint>

namespace mods
{
   namespace utils
     {
        template<typename T>
          auto at(const T& collection, std::size_t index) -> const typename T::value_type&
            {
               assert(index >= 0 && index < collection.size());
               return collection[index]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
            }
        
        template<typename T>
          auto at(T& collection, std::size_t index) -> typename T::value_type&
            {
               assert(index >= 0 && index < collection.size());
               return collection[index]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
            }
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_AT_HPP