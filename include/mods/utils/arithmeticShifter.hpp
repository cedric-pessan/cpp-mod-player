#ifndef MODS_UTILS_ARITHMETICSHIFTER_HPP
#define MODS_UTILS_ARITHMETICSHIFTER_HPP

#include "mods/utils/types.hpp"

namespace mods
{
   namespace utils
     {
        namespace arithmeticShifter
          {
             inline auto shiftRight(s32 value, u32 n) -> s32
               {
                  u32 x = static_cast<u32>(value);
                  if(value < 0)
                    {
                       return x >> n | ~(~0U >> n);
                    }
                  return x >> n;
               }
          } // namespace arithmeticShifter
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_ARITHMETICSHIFTER_HPP
