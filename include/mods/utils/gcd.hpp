#ifndef MODS_UTILS_GCD_HPP
#define MODS_UTILS_GCD_HPP

#include "mods/utils/types.hpp"

#include <limits>

namespace mods
{
   namespace utils
     {
        constexpr auto gcd(u64 paramA, u64 paramB) -> u64
          {
             u64 remaining = std::numeric_limits<u64>::max();
             while(remaining != 0)
               {
                  if(paramA < paramB)
                    {
                       const u64 tmp = paramA;
                       paramA = paramB;
                       paramB = tmp;
                    }
                  remaining = paramA % paramB;
                  if(remaining != 0)
                    {
                       paramA = paramB;
                       paramB = remaining;
                    }
               }
             return paramB;
          }
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_GCD_HPP
