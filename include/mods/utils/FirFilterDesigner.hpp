#ifndef MODS_UTILS_FIRFILTERDESIGNER_HPP
#define MODS_UTILS_FIRFILTERDESIGNER_HPP

#include "mods/utils/types.hpp"

#include <string>
#include <vector>

namespace mods
{
   namespace utils
     {
        class FirFilterDesigner
          {
           public:
	     FirFilterDesigner(u64 sampleFrequency, double cutOff, double expectedAttenuation, double transitionWidth);
             
             FirFilterDesigner() = delete;
             FirFilterDesigner(const FirFilterDesigner&) = default;
             FirFilterDesigner(FirFilterDesigner&&) = default;
             auto operator=(const FirFilterDesigner&) -> FirFilterDesigner& = delete;
             auto operator=(FirFilterDesigner&&) -> FirFilterDesigner& = delete;
             ~FirFilterDesigner() = default;
             
             using TapsType = std::vector<double>;
             
             auto getTaps() const -> const TapsType&;
             
           private:
             static void checkInit(bool condition, const std::string& description);
             
             void computeFilter();
             
	     double _sampleFrequency;
	     double _cutOff;
             double _expectedAttenuation;
             double _transitionWidth;
	     
	     TapsType _taps;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_FIRFILTERDESIGNER_HPP
