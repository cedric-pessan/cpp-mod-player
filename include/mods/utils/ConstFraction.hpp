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
             ConstFraction& operator=(const ConstFraction&) = delete;
             ConstFraction& operator=(ConstFraction&&) = delete;
             ~ConstFraction() = default;
             
             constexpr ConstFraction reduce() const
               {
                  int factor = gcd(_numerator, _denominator);
                  return {_numerator / factor, 
                          _denominator / factor};
               }
             
             constexpr int getDenominator() const
               {
                  return _denominator;
               }
             
             constexpr int getNumerator() const
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
