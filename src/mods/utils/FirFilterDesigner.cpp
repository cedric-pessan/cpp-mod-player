
#include "mods/utils/FirFilterDesigner.hpp"
#include "mods/utils/MathConstants.hpp"
#include "mods/utils/bessel.hpp"
#include "mods/utils/types.hpp"

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace mods
{
   namespace utils
     {
        namespace
          {
             constexpr double minAttenuation = 21.0;
          } // namespace
        
	FirFilterDesigner::FirFilterDesigner(const FirFilterDesigner::Params& params)
	  : _sampleFrequency(static_cast<double>(params.sampleFrequency)),
	  _cutOff(params.cutOff),
          _expectedAttenuation(params.expectedAttenuation),
          _transitionWidth(params.transitionWidth)
	    {
               checkInit(params.transitionWidth > 0, "transition width should be positive");
               checkInit(params.expectedAttenuation > minAttenuation, "attenuation should be greater than 8dB");
               computeFilter();
	    }
        
        void FirFilterDesigner::checkInit(bool condition, const std::string& description)
          {
             if(!condition)
               {
                  throw std::invalid_argument(description);
               }
          }
	
	void FirFilterDesigner::computeFilter()
	  {
             using mods::utils::math::cPI;
             
             static constexpr double two = 2.0;
             static constexpr double highAttenuationLimit = 50.0;
             
	     const double nyquistFrequency = _sampleFrequency / two;
	     const double deltaOmega = _transitionWidth / nyquistFrequency * cPI;
             // apply empirical kaiser window formula to determine number of taps
	     s64 MTaps = std::lround((_expectedAttenuation - 8.0) / (2.285 * deltaOmega)); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	     if((static_cast<u64>(MTaps)&1U) == 0)
               {
                  ++MTaps;
               }
	     
	     const double omegaC = _cutOff / _sampleFrequency;
	     
	     for(s64 i=0; i<MTaps; ++i)
	       {
		  if((i-(MTaps/2)) == 0)
		    {
		       _taps.push_back(omegaC * two);
		    }
		  else
		    {
                       const s64 shiftedFrequency = i-(MTaps/2);
                       auto omega = static_cast<double>(shiftedFrequency);
		       _taps.push_back(std::sin(two * cPI * omegaC * omega) / (cPI * omega));
		    }
	       }
	     
	     // kaiser window
	     
	     // compute beta parameter
	     double beta = 0.0;
	     if(_expectedAttenuation > highAttenuationLimit)
	       {
		  beta = 0.1102 * (_expectedAttenuation - 8.7); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	       }
	     else if(_expectedAttenuation > minAttenuation)
	       {
		  beta = 0.5842 * std::pow(_expectedAttenuation - minAttenuation, 0.4) + 0.07886 * (_expectedAttenuation - minAttenuation); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	       }
	     
	     // compute and apply kaiser window
	     const double alpha = static_cast<double>(MTaps) / two;
	     for(s64 i=0; i<MTaps; ++i)
	       {
                  const s64 shiftedFrequency = i-(MTaps/2);
                  auto omega = static_cast<double>(shiftedFrequency);
		  const double kaiserValue = bessel::i0(beta * std::sqrt(1.0 - std::pow(omega / alpha, 2))) / bessel::i0(beta);
		  
		  _taps[i] *= kaiserValue;
	       }
	  }
        
        auto FirFilterDesigner::getTaps() const -> const std::vector<double>&
          {
             return _taps;
          }
     } // namespace utils
} // namespace mods
