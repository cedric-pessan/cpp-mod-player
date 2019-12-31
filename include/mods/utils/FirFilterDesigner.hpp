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
	     
             explicit FirFilterDesigner(std::vector<Band> bands);
             
             FirFilterDesigner() = delete;
             FirFilterDesigner(const FirFilterDesigner&) = delete;
             FirFilterDesigner(FirFilterDesigner&&) = delete;
             FirFilterDesigner& operator=(const FirFilterDesigner&) = delete;
             FirFilterDesigner& operator=(FirFilterDesigner&&) = delete;
             ~FirFilterDesigner() = default;
             
             void optimizeFilter(int desiredTaps);
             void displayProgress();
             
             const std::vector<double>& getTaps() const;
             
           private:
             double getLinearFreq(double w) const;
             void initFrequencies();
             double getDesiredGain(double w) const;
             bool isInLimits(double w) const;
             double getWeight(double w) const;
             double getActualGain(double w) const;
             double getError(double w) const;
             void calcDelta();
             double getOnlyDelta();
             bool getExtremaAdaptively();
             int getExtrema(int resolution);
             void updateFrequencies();
             void designFilter();
             void buildTaps();
             
             int binarySearch(int min, int max, const std::function<bool(int)>& isGood) const;
             
             int _numTaps = 0;
             int _r = 0;
             double _delta = 0.0;
             double _sumBands = 0.0;
             bool _requirementsMet = false;
             bool _noConvergence = false;
             double _maxError = 0.0;
             bool _displayProgress = false;
             
             std::vector<double> _frequencies;
             std::vector<double> _x;
             std::vector<double> _y;
             std::vector<double> _di;
             std::vector<double> _dn;
             
             std::vector<double> _dy;
             std::vector<double> _w;
             std::vector<double> _d;
             
             std::vector<Band> _bands;
             std::vector<double> _taps;
             
             class Extremum
               {
                public:
                  Extremum(double freq, double error);
                  
                  Extremum() = delete;
                  Extremum(const Extremum&) = default;
                  Extremum(Extremum&&) = default;
                  Extremum& operator=(const Extremum&) = default;
                  Extremum& operator=(Extremum&&) = default;
                  ~Extremum() = default;
                  
                  double getFreq() const;
                  double getError() const;
                  
                private:
                  double _freq;
                  double _error;
               };
             
             std::list<Extremum> _extrema;
             std::list<Extremum> _allExtrema;
             
             double makeAlternating(double firstOneSign, std::list<Extremum>* candidates) const;
             double calcMaxError(std::list<Extremum>* extrema) const;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_FIRFILTERDESIGNER_HPP
