
#include "mods/utils/bessel.hpp"
#include "mods/utils/FirFilterDesigner.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>

namespace mods
{
   namespace utils
     {
	FirFilterDesigner::FirFilterDesigner(int sampleFrequency, double cutOff)
	  : _sampleFrequency(sampleFrequency),
	  _cutOff(cutOff)
	    {
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
	
        /*FirFilterDesigner::FirFilterDesigner(std::vector<Band> bands)
          : _bands(std::move(bands))
            {
            }
        
        void FirFilterDesigner::optimizeFilter(int desiredTaps)
          {
             _numTaps = 21;
             
             double deltaLimit = 1e-12;
             
             while(true)
               {
                  double delta = std::abs(getOnlyDelta());
                  if(delta < deltaLimit && delta > 0.0)
                    {
                       break;
                    }
                  
                  _numTaps = _numTaps + _numTaps / 2;
                  
                  if(_numTaps > 2000)
                    {
                       break;
                    }
               }
             
             int opt = binarySearch(1, _numTaps, [this,deltaLimit](int c) -> bool {
                _numTaps = c;
                return std::abs(getOnlyDelta()) > deltaLimit;
             }) + 2;
             
             if(desiredTaps != -1)
               {
                  if(opt <= desiredTaps)
                    {
                       throw std::runtime_error("Precision exceeded");
                    }
               }
             
             if(desiredTaps == -1)
               {
                  opt = binarySearch(1, opt, [this](int c) -> bool {
                     _numTaps = c;
                     designFilter();
                     return !_requirementsMet || _noConvergence;
                  });
                  _numTaps = opt + 2;
               }
             else
               {
                  _numTaps = desiredTaps + 2;
               }
             
             designFilter();
             buildTaps();
          }*/
        
        const std::vector<double>& FirFilterDesigner::getTaps() const
          {
             return _taps;
          }
        
        /*void FirFilterDesigner::displayProgress()
          {
             _displayProgress = true;
          }
        
        double FirFilterDesigner::getLinearFreq(double w) const
          {
             for(auto b : _bands)
               {
                  if(w < b.getStop() - b.getStart())
                    {
                       return b.getStart() + w;
                    }
                  
                  w -= b.getStop() - b.getStart();
               }
             return 0.0;
          }
        
        void FirFilterDesigner::initFrequencies()
          {
             _sumBands = 0.0;
             
             _frequencies.clear();
             
             for(auto& b : _bands)
               {
                  _sumBands += b.getStop() - b.getStart();
               }
             
             double step = _sumBands / (_r+2);
             
             for(int i=0; i<_r+1; ++i)
               {
                  _frequencies.push_back(getLinearFreq(step*(i+1)));
               }
          }
        
        double FirFilterDesigner::getDesiredGain(double w) const
          {
             for(auto b : _bands)
               {
                  if(w >= b.getStart() && w <= b.getStop())
                    {
                       return (_numTaps % 2 != 0) ? b.getGain() : b.getGain() / std::cos(w * M_PI / 2.0);
                    }
               }
             return 0.0;
          }
        
        bool FirFilterDesigner::isInLimits(double w) const
          {
             for(auto& b : _bands)
               {
                  if(w >= b.getStart() && w <= b.getStop())
                    {
                       double gain = std::abs(getActualGain(w));
                       
                       if(b.getGain() != 0.0)
                         {
                            return gain >= b.getLower() && gain <= b.getUpper();
                         }
                       
                       return gain <= b.getUpper();
                    }
               }
             
             return true;
          }
        
        double FirFilterDesigner::getWeight(double w) const
          {
             for(auto& b : _bands)
               {
                  if(w >= b.getStart() && w <= b.getStop())
                    {
                       return (_numTaps % 2 != 0) ? b.getWeight() : b.getWeight() * std::cos(w * M_PI / 2.0);
                    }
               }
             return 0.0;
          }
        
        double FirFilterDesigner::getActualGain(double w) const
          {
             double numerator = 0.0;
             double denominator = 0.0;
             double cosw = std::cos(w * M_PI);
             
             for(size_t i=0; i<_frequencies.size(); ++i)
               {
                  double c = cosw - _x[i];
                  if(std::abs(c) < 1e-12)
                    {
                       numerator = _y[i];
                       denominator = 1.0;
                       break;
                    }
                  c = _dn[i] / c;
                  denominator += c;
                  numerator += c * _y[i];
               }
             
             return numerator / denominator;
          }
        
        void FirFilterDesigner::buildTaps()
          {
             std::vector<double> taps;
             std::vector<double> h;
             
             for(int i=0; i<=_numTaps/2; ++i)
               {
                  double c;
                  if(_numTaps % 2 == 0)
                    {
                       c = std::cos(M_PI * i / static_cast<double>(_numTaps));
                    }
                  else
                    {
                       c = 1.0;
                    }
                  taps.push_back(getActualGain(i * 2.0 / _numTaps) * c);
               }
             
             if(_displayProgress)
               {
                  std::cout << "taps: ";
                  for(double d : taps)
                    {
                       std::cout << d << ',';
                    }
                  
                  std::cout << std::endl;
               }
             
             double M = (_numTaps - 1) / 2.0;
             
             if(_numTaps % 2 == 0)
               {
                  for(int i=1; i<_numTaps-1; ++i)
                    {
                       double val = taps[0];
                       double x = M_PI * 2.0 * (double(i)-M)/_numTaps;
                       for(int k=1; k<=_numTaps/2 - 1; ++k)
                         {
                            val += 2.0 * taps[k] * std::cos(x*k);
                         }
                       
                       h.push_back(val / _numTaps);
                    }
               }
             else
               {
                  for(int i=1; i<_numTaps-1; ++i)
                    {
                       double val = taps[0];
                       double x = M_PI * 2.0 * (double(i)-M)/_numTaps;
                       for(int k=1; k<=M; ++k)
                         {
                            val += 2.0 * taps[k] * std::cos(x*k);
                         }
                       
                       h.push_back(val / _numTaps);
                    }
               }
             _taps = std::move(h);
          }
        
        double FirFilterDesigner::getError(double w) const
          {
             return getWeight(w) * (getDesiredGain(w) - getActualGain(w));
          }
        
        void FirFilterDesigner::calcDelta()
          {
             _x.clear();
             _y.clear();
             _di.clear();
             _dn.clear();
             double numerator = 0.0;
             double denominator = 0.0;
             
             for(auto freq : _frequencies)
               {
                  _x.push_back(std::cos(freq * M_PI));
               }
             
             for(size_t i=0; i < _x.size(); ++i)
               {
                  _di.push_back(1.0);
                  for(size_t j=0; j < _x.size(); ++j)
                    {
                       if(i != j)
                         {
                            _di[i] *= (_x[j] - _x[i]);
                         }
                    }
                  _dn.push_back(1.0/_di[i]);
               }
             
             double sign = 1.0;
             for(size_t i=0; i < _frequencies.size(); ++i)
               {
                  numerator += getDesiredGain(_frequencies[i]) * _dn[i];
                  denominator += sign / getWeight(_frequencies[i]) * _dn[i];
                  sign = -sign;
               }
             
             _delta = numerator / denominator;
             
             _dy.clear();
             _w.clear();
             _d.clear();
             sign = 1.0;
             
             for(size_t i=0; i<_frequencies.size(); ++i)
               {
                  _w.push_back(getWeight(_frequencies[i]));
                  _dy.push_back(sign * _delta / _w[i]);
                  _d.push_back(getDesiredGain(_frequencies[i]));
                  _y.push_back(_d[i] - _dy[i]);
                  
                  sign = -sign;
               }
          }
        
        double FirFilterDesigner::getOnlyDelta()
          {
             if((_numTaps % 2) == 0)
               {
                  _r = _numTaps / 2;
               }
             else
               {
                  _r = (_numTaps + 1) / 2;
               }
             
             initFrequencies();
             calcDelta();
             
             return _delta;
          }
        
        double FirFilterDesigner::calcMaxError(std::list<Extremum>* extrema) const
          {
             double maxError = 0.0;
             for(auto& extremum : *extrema)
               {
                  if(std::abs(extremum.getError()) > maxError)
                    {
                       maxError = std::abs(extremum.getError());
                    }
               }
             
             return maxError;
          }
        
        int FirFilterDesigner::getExtrema(int resolution)
          {
             _extrema.clear();
             std::vector<Extremum> candidates;
             
             std::vector<double> E;
             std::vector<double> F;
             
             for(int i=0; i<resolution; ++i)
               {
                  F.push_back(getLinearFreq(_sumBands*(i+1.0)/(resolution+1.0)));
                  E.push_back(getError(F[i]));
               }
             
             if((E[0] > 0.0 && E[0] > E[1]) ||
                (E[0] < 0.0 && E[0] < E[1]))
               {
                  candidates.emplace_back(Extremum(F[0], E[0]));
               }
             
             int j = resolution - 1;
             if((E[j] > 0.0 && E[j] > E[j-1]) ||
                (E[j] < 0.0 && E[j] < E[j-1]))
               {
                  candidates.emplace_back(Extremum(F[j], E[j]));
               }
             
             for(int i=1; i<resolution-1; ++i)
               {
                  if((E[i] >= E[i-1] && E[i] > E[i+1] && E[i] > 0.0) ||
                     (E[i] <= E[i-1] && E[i] < E[i+1] && E[i] < 0.0))
                    {
                       candidates.emplace_back(Extremum(F[i], E[i]));
                    }
               }
             
             for(auto& b : _bands)
               {
                  candidates.emplace_back(Extremum(b.getStart(), getError(b.getStart())));
                  candidates.emplace_back(Extremum(b.getStop(), getError(b.getStop())));
               }
             
             std::sort(candidates.begin(), candidates.end(), [](const Extremum& a, const Extremum& b) {
                return a.getFreq() < b.getFreq();
             });
             
             _allExtrema.clear();
             std::list<Extremum> firstUp;
             std::list<Extremum> firstDown;
             
             for(auto& candidate : candidates)
               {
                  _allExtrema.push_back(candidate);
                  firstUp.push_back(candidate);
                  firstDown.push_back(candidate);
               }
             
             double firstUpError = makeAlternating(1.0, &firstUp);
             double firstDownError = makeAlternating(-1.0, &firstDown);
             
             if(firstUpError > firstDownError)
               {
                  _extrema = firstUp;
               }
             else
               {
                  _extrema = firstDown;
               }
             
             _maxError = calcMaxError(&_allExtrema);
             
             return _extrema.size();
          }
        
        void FirFilterDesigner::updateFrequencies()
          {
             _frequencies.clear();
             std::vector<Extremum> e(_extrema.begin(), _extrema.end());
             std::sort(e.begin(), e.end(), [](const Extremum& a, const Extremum& b) {
                return std::abs(a.getError()) > std::abs(b.getError());
             });
             for(auto& extremum : e)
               {
                  _frequencies.push_back(extremum.getFreq());
                  if(_frequencies.size() == static_cast<size_t>(_r)+1)
                    {
                       break;
                    }
               }
             std::sort(_frequencies.begin(), _frequencies.end(), [](double a, double b) {
                return std::abs(a) < std::abs(b);
             });
          }
        
        double FirFilterDesigner::makeAlternating(double firstOneSign, std::list<Extremum>* candidates) const
          {
             while(!candidates->empty() && candidates->begin()->getError() * firstOneSign < 0.0)
               {
                  candidates->erase(candidates->begin());
               }
             
             size_t req_length = _r+1;
             while( candidates->size() > req_length)
               {
                  for(auto it = ++candidates->begin(); it != candidates->end(); ++it)
                    {
                       if(it->getError() * std::prev(it)->getError() >= 0.0)
                         {
                            if(std::abs(it->getError()) > std::abs(std::prev(it)->getError()))
                              {
                                 candidates->erase(std::prev(it));
                              }
                            else
                              {
                                 candidates->erase(it--);
                              }
                         }
                    }
                  
                  if(candidates->size() == req_length+1)
                    {
                       if(std::abs(candidates->front().getError()) > std::abs(candidates->back().getError()))
                         {
                            candidates->pop_back();
                         }
                       else
                         {
                            candidates->pop_front();
                         }
                    }
                  else if(candidates->size() > req_length)
                    {
                       auto min = candidates->begin();
                       for(auto it = candidates->begin(); it != candidates->end(); ++it)
                         {
                            if(std::abs(it->getError()) < std::abs(min->getError()))
                              {
                                 min = it;
                              }
                         }
                       candidates->erase(min);
                    }
               }
             
             if(candidates->size() < req_length)
               {
                  return -1.0;
               }
             
             return calcMaxError(candidates);
          }
        
        bool FirFilterDesigner::getExtremaAdaptively()
          {
             int resolution = _r * 16;
             int num_extrema = 0;
             
             while(num_extrema < _r+1)
               {
                  if(resolution > 200000)
                    {
                       return false;
                    }
                  
                  num_extrema = getExtrema(resolution);
                  if(_displayProgress)
                    {
                       std::cout << "resolution = " << resolution << std::endl;
                    }
                  resolution *= 10;
               }
             return true;
          }
        
        void FirFilterDesigner::designFilter()
          {
             _noConvergence = false;
             _requirementsMet = false;
             
             if(_numTaps % 2 == 0)
               {
                  _r = _numTaps / 2;
               }
             else
               {
                  _r = (_numTaps + 1) / 2;
               }
             
             initFrequencies();
             
             double lastError = 1.0;
             int maxIter = 20;
             int iter;
             for(iter = 0; iter < maxIter; iter++)
               {
                  calcDelta();
                  
                  if(!getExtremaAdaptively())
                    {
                       _noConvergence = true;
                       return;
                    }
                  
                  double errorChange = lastError - _maxError;
                  
                  if(_displayProgress)
                    {
                       std::cout << "error = " << _maxError << ", errorchange = " << errorChange << std::endl;
                       std::cout << std::endl;
                    }
                  if(errorChange >= 0.0 && errorChange < lastError * 0.001 && errorChange < 1E-10)
                    {
                       break;
                    }
                  
                  updateFrequencies();
                  lastError = _maxError;
               }
             
             _requirementsMet = true;
             for(auto& extremum : _allExtrema)
               {
                  if(!isInLimits(extremum.getFreq()))
                    {
                       _requirementsMet = false;
                       break;
                    }
               }
             
             if(!_requirementsMet && iter == maxIter)
               {
                  _noConvergence = true;
               }
          }
        
        int FirFilterDesigner::binarySearch(int min, int max, const std::function<bool(int)>& isGood) const
          {
             while(true)
               {
                  int c = (max + min) / 2;
                  if(c % 2 == 0)
                    {
                       c++;
                    }
                  
                  if(isGood(c))
                    {
                       min = c;
                    }
                  else
                    {
                       max = c;
                    }
                  
                  if(max - min < 3)
                    {
                       return min;
                    }
               }
          }
        
        FirFilterDesigner::Extremum::Extremum(double freq, double error)
          : _freq(freq),
          _error(error)
            {
            }
        
        double FirFilterDesigner::Extremum::getFreq() const
          {
             return _freq;
          }
        
        double FirFilterDesigner::Extremum::getError() const
          {
             return _error;
          }*/
     } // namespace utils
} // namespace mods
