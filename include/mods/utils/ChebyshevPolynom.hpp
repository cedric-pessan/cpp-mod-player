#ifndef MODS_UTILS_CHEBYSHEVPOLYNOM_HPP
#define MODS_UTILS_CHEBYSHEVPOLYNOM_HPP

namespace mods
{
   namespace utils
     {
        namespace chebyshevPolynom
          {
             template<typename CoefList>
               auto eval(double x, const CoefList& coefficients) -> double
                 {
                    auto it = coefficients.begin();
                    auto b0 = *it;
                    ++it;
                    
                    double b1 = 0.0;
                    double b2 = 0.0;
                    
                    for(;it != coefficients.end(); ++it)
                      {
                         b2 = b1;
                         b1 = b0;
                         b0 = x * b1 - b2 + *it;
                      }
                    
                    static constexpr double const_0_5 = 0.5;
                    return const_0_5 * (b0 - b2);
                 }
          } // namespace chebyshevPolynom
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_CHEBYSHEVPOLYNOM_HPP
