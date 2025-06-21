#ifndef MODS_UTILS_CHEBYSHEVPOLYNOM_HPP
#define MODS_UTILS_CHEBYSHEVPOLYNOM_HPP

namespace mods
{
   namespace utils
     {
        namespace chebyshevPolynom
          {
             template<typename CoefList>
               auto eval(double inputValue, const CoefList& coefficients) -> double
                 {
                    auto itCoef = coefficients.begin();
                    auto valB0 = *itCoef;
                    ++itCoef;
                    
                    double valB1 = 0.0;
                    double valB2 = 0.0;
                    
                    for(;itCoef != coefficients.end(); ++itCoef)
                      {
                         valB2 = valB1;
                         valB1 = valB0;
                         valB0 = inputValue * valB1 - valB2 + *itCoef;
                      }
                    
                    static constexpr double const_0_5 = 0.5;
                    return const_0_5 * (valB0 - valB2);
                 }
          } // namespace chebyshevPolynom
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_CHEBYSHEVPOLYNOM_HPP
