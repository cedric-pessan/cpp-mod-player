
#include "mods/utils/bessel.hpp"
#include "mods/utils/FirFilterDesigner.hpp"

#include <cmath>

namespace mods
{
   namespace utils
     {
	FirFilterDesigner::FirFilterDesigner(u32 sampleFrequency, double cutOff)
	  : _sampleFrequency(sampleFrequency),
	  _cutOff(cutOff)
	    {
               computeFilter();
	    }
        
        namespace
          {
             constexpr double transitionWidth = 50.0;
             constexpr double expectedAttenuation = 40.0;
          } // namespace
	
	void FirFilterDesigner::computeFilter()
	  {
             static constexpr double two = 2.0;
             static constexpr double minAttenuation = 21.0;
             static constexpr double highAttenuationLimit = 50.0;
             
             double A = expectedAttenuation;
	     double nyquistFrequency = _sampleFrequency / two;
	     double deltaOmega = transitionWidth / nyquistFrequency * M_PI;
             // apply empirical kaiser window formula to determine number of taps
             static_assert(transitionWidth > 0, "transition width should be positive");
             static_assert(expectedAttenuation > minAttenuation, "attenuation should be greater than 8dB");
	     s64 M = std::lround((A - 8.0) / (2.285 * deltaOmega)); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	     if((static_cast<u64>(M)&1U) == 0)
               {
                  ++M;
               }
	     
	     double wc = _cutOff / _sampleFrequency;
	     
	     for(s64 i=0; i<M; ++i)
	       {
		  if((i-(M/2)) == 0)
		    {
		       _taps.push_back(wc * two);
		    }
		  else
		    {
                       s64 shiftedFrequency = i-M/2;
                       auto w = static_cast<double>(shiftedFrequency);
		       _taps.push_back(std::sin(two * M_PI * wc * w) / (M_PI * w));
		    }
	       }
	     
	     // kaiser window
	     
	     // compute beta parameter
	     double beta = 0.0;
	     if(A > highAttenuationLimit)
	       {
		  beta = 0.1102 * (A - 8.7); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	       }
	     else if(A > minAttenuation)
	       {
		  beta = 0.5842 * std::pow(A - minAttenuation, 0.4) + 0.07886 * (A - minAttenuation); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
	       }
	     
	     // compute and apply kaiser window
	     double alpha = M / two;
	     for(s64 i=0; i<M; ++i)
	       {
		  namespace bessel = mods::utils::bessel;
                  s64 shiftedFrequency = i-M/2;
                  auto w = static_cast<double>(shiftedFrequency);
		  double kaiserValue = bessel::i0(beta * std::sqrt(1.0 - std::pow(w / alpha, 2))) / bessel::i0(beta);
		  
		  _taps[i] *= kaiserValue;
	       }
	  }
        
        auto FirFilterDesigner::getTaps() const -> const std::vector<double>&
          {
             return _taps;
          }
     } // namespace utils
} // namespace mods
