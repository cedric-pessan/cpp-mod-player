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
             
             auto getTaps() const -> const std::vector<double>&;
             
           private:
             void optimizeFilter();
             
	     double _sampleFrequency;
	     double _cutOff;
	     
	     std::vector<double> _taps;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_FIRFILTERDESIGNER_HPP
