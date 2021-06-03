#ifndef MODS_UTILS_GCD_HPP
#define MODS_UTILS_GCD_HPP

#include "mods/utils/types.hpp"

namespace mods
{
   namespace utils
     {
        constexpr auto gcd(u32 a, u32 b) -> u32
          {
             u32 r = 0;
             do
               {
                  if(a < b)
                    {
                       u32 t = a;
                       a = b;
                       b = t;
                    }
                  r = a % b;
                  if(r != 0)
                    {
                       a = b;
                       b = r;
                    }
               } while(r != 0);
             return b;
          }
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_GCD_HPP
