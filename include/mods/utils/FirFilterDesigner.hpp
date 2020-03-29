#ifndef MODS_UTILS_FIRFILTERDESIGNER_HPP
#define MODS_UTILS_FIRFILTERDESIGNER_HPP

#include <vector>

namespace mods
{
   namespace utils
     {
        class FirFilterDesigner
          {
           public:
	     FirFilterDesigner(int sampleFrequency, double cutOff);
             
             FirFilterDesigner() = delete;
             FirFilterDesigner(const FirFilterDesigner&) = default;
             FirFilterDesigner(FirFilterDesigner&&) = default;
             FirFilterDesigner& operator=(const FirFilterDesigner&) = delete;
             FirFilterDesigner& operator=(FirFilterDesigner&&) = delete;
             ~FirFilterDesigner() = default;
             
             const std::vector<double>& getTaps() const;
             
           private:
             void optimizeFilter();
             
	     double _sampleFrequency;
	     double _cutOff;
	     
	     std::vector<double> _taps;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_FIRFILTERDESIGNER_HPP
