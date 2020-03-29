
#include "mods/utils/bessel.hpp"
#include "mods/utils/FirFilterDesigner.hpp"

#include <cmath>

namespace mods
{
   namespace utils
     {
	FirFilterDesigner::FirFilterDesigner(int sampleFrequency, double cutOff)
	  : _sampleFrequency(sampleFrequency),
	  _cutOff(cutOff)
	    {
               optimizeFilter();
	    }
	
	void FirFilterDesigner::optimizeFilter()
	  {
	     double A = 40.0;
	     double transitionWidth = 50.0;
	     double niquist = _sampleFrequency / 2.0;
	     double deltaOmega = transitionWidth / niquist * M_PI;
	     int M = static_cast<int>(((A - 8) / (2.285 * deltaOmega)) + 0.5);
	     if((M&1) == 0) ++M;
	     
	     double wc = _cutOff / _sampleFrequency;
	     
	     for(int i=0; i<M; ++i)
	       {
		  if((i-(M/2)) == 0)
		    {
		       _taps.push_back(wc * 2.0);
		    }
		  else
		    {
		       _taps.push_back(std::sin(2.0 * M_PI * wc * (i-M/2)) / (M_PI * (i-(M/2))));
		    }
	       }
	     
	     // kaiser window
	     
	     // compute beta parameter
	     double beta = 0.0;
	     if(A > 50)
	       {
		  beta = 0.1102 * (A - 8.7);
	       }
	     else if(A > 21)
	       {
		  beta = 0.5842 * std::pow(A - 21.0, 0.4) + 0.07886 * (A - 21.0);
	       }
	     
	     // compute and apply kaiser window
	     double alpha = M / 2.0;
	     for(int i=0; i<M; ++i)
	       {
		  namespace bessel = mods::utils::bessel;
		  double kaiserValue = bessel::i0(beta * std::sqrt(1.0 - std::pow((i - alpha) / alpha, 2.0))) / bessel::i0(beta);
		  
		  _taps[i] *= kaiserValue;
	       }
	  }
        
        const std::vector<double>& FirFilterDesigner::getTaps() const
          {
             return _taps;
          }
     } // namespace utils
} // namespace mods
