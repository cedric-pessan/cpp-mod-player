#ifndef MODS_UTILS_FIRFILTERDESIGNER_HPP
#define MODS_UTILS_FIRFILTERDESIGNER_HPP

#include <functional>
#include <list>
#include <memory>
#include <vector>

#include "mods/utils/Band.hpp"

namespace mods
{
   namespace utils
     {
        class FirFilterDesigner
          {
           public:
	     using ptr = std::unique_ptr<FirFilterDesigner>;
	     
	     FirFilterDesigner(int sampleFrequency, double cutOff);
             
             FirFilterDesigner() = delete;
             FirFilterDesigner(const FirFilterDesigner&) = delete;
             FirFilterDesigner(FirFilterDesigner&&) = delete;
             FirFilterDesigner& operator=(const FirFilterDesigner&) = delete;
             FirFilterDesigner& operator=(FirFilterDesigner&&) = delete;
             ~FirFilterDesigner() = default;
             
             void optimizeFilter();
             
             const std::vector<double>& getTaps() const;
             
           private:
	     double _sampleFrequency;
	     double _cutOff;
	     
	     std::vector<double> _taps;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_FIRFILTERDESIGNER_HPP
