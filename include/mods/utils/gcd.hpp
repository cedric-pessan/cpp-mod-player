#ifndef MODS_UTILS_GCD_HPP
#define MODS_UTILS_GCD_HPP

#include "mods/utils/types.hpp"

namespace mods
{
   namespace utils
     {
        constexpr auto gcd(u64 a, u64 b) -> u64
          {
             u64 r = 0;
             do
               {
                  if(a < b)
                    {
                       u64 t = a;
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
