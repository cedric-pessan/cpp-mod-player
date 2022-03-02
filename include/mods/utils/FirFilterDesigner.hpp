#ifndef MODS_UTILS_FIRFILTERDESIGNER_HPP
#define MODS_UTILS_FIRFILTERDESIGNER_HPP

#include "mods/utils/types.hpp"

#include <vector>

namespace mods
{
   namespace utils
     {
        class FirFilterDesigner
          {
           public:
	     FirFilterDesigner(u32 sampleFrequency, double cutOff);
             
             FirFilterDesigner() = delete;
             FirFilterDesigner(const FirFilterDesigner&) = default;
             FirFilterDesigner(FirFilterDesigner&&) = default;
             auto operator=(const FirFilterDesigner&) -> FirFilterDesigner& = delete;
             auto operator=(FirFilterDesigner&&) -> FirFilterDesigner& = delete;
             ~FirFilterDesigner() = default;
             
             using TapsType = std::vector<double>;
             
             auto getTaps() const -> const TapsType&;
             
           private:
             void computeFilter();
             
	     double _sampleFrequency;
	     double _cutOff;
	     
	     TapsType _taps;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_FIRFILTERDESIGNER_HPP
