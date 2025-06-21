#ifndef MODS_UTILS_ARITHMETICSHIFTER_HPP
#define MODS_UTILS_ARITHMETICSHIFTER_HPP

#include "mods/utils/types.hpp"

namespace mods
{
   namespace utils
     {
        namespace arithmeticShifter
          {
             enum Shift : u32;
             
             template<typename T>
               inline auto shiftRight(T value, Shift n) -> T
               {
                  static_assert(std::is_arithmetic<T>::value, "T should be a numerical type");
                  static_assert(std::is_signed<T>::value, "T should be signed");
                  using UT = typename std::make_unsigned_t<T>;
                  
                  const UT unsignedValue = static_cast<UT>(value);
                  if(value < 0)
                    {
                       return static_cast<T>(static_cast<UT>(unsignedValue >> n) | 
                                             static_cast<UT>(~static_cast<UT>(static_cast<UT>(~0U) >> n)));
                    }
                  return static_cast<T>(unsignedValue >> n);
               }
          } // namespace arithmeticShifter
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_ARITHMETICSHIFTER_HPP
