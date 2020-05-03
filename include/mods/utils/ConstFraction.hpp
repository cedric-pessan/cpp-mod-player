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
             constexpr ConstFraction(int numerator, int denominator)
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
                  int factor = gcd(_numerator, _denominator);
                  return {_numerator / factor, 
                          _denominator / factor};
               }
             
             constexpr auto getDenominator() const -> int
               {
                  return _denominator;
               }
             
             constexpr auto getNumerator() const -> int
               {
                  return _numerator;
               }
             
           private:
             int _numerator;
             int _denominator;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_CONSTFRACTION_HPP
