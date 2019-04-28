#ifndef MODS_UTILS_GCD_HPP
#define MODS_UTILS_GCD_HPP

namespace mods
{
   namespace utils
     {
        constexpr int gcd(int a, int b)
          {
             if(a < b)
               {
                  return gcd(b, a);
               }
             int r = a % b;
             if(r == 0)
               {
                  return b;
               }
             return gcd(b, r);
          }
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_GCD_HPP
