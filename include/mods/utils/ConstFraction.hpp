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
             enum Numerator : u64;
             enum Denominator : u64;
             
             constexpr ConstFraction(Numerator numerator, Denominator denominator)
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
                  const u64 factor = gcd(_numerator, _denominator);
                  return {static_cast<Numerator>(_numerator / factor), 
                          static_cast<Denominator>(_denominator / factor)};
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
