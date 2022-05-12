#ifndef MODS_UTILS_CONSTFRACTION_HPP
#define MODS_UTILS_CONSTFRACTION_HPP

#include "mods/utils/gcd.hpp"

namespace mods
{
   namespace utils
     {
        class ConstFraction
          {
           public:
             constexpr ConstFraction(u64 numerator, u64 denominator)
               : _numerator(numerator),
               _denominator(denominator)
                 {
                 }
             
             ConstFraction() = delete;
             ConstFraction(const ConstFraction&) = default;
             ConstFraction(ConstFraction&&) = default;
             auto operator=(const ConstFraction&) -> ConstFraction& = delete;
             auto operator=(ConstFraction&&) -> ConstFraction& = delete;
             ~ConstFraction() = default;
             
             constexpr auto reduce() const -> ConstFraction
               {
                  u64 factor = gcd(_numerator, _denominator);
                  return {_numerator / factor, 
                          _denominator / factor};
               }
             
             constexpr auto getDenominator() const -> u64
               {
                  return _denominator;
               }
             
             constexpr auto getNumerator() const -> u64
               {
                  return _numerator;
               }
             
           private:
             u64 _numerator;
             u64 _denominator;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_CONSTFRACTION_HPP
