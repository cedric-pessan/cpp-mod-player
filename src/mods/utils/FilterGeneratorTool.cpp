
#include <iostream>
#include <fstream>
#include <vector>

#include "mods/utils/ConstFraction.hpp"
#include "mods/utils/FirFilterDesigner.hpp"

namespace mods
{
   namespace utils
     {
        struct LowPassParam
          {
             int cutoffFrequency;
             int cutoffFrequencyDivider;
             int sampleFrequency;
             int numTaps;
          };
        
        std::vector<LowPassParam> lowPassParams
          {
               { 22000, 2, 22000 * ConstFraction(22000,44100).reduce().getDenominator(), 1545 } // 22kHz -> 44100Hz
          };
        
        std::string generateModuleName(const std::string& filename)
          {
             size_t pos = 0;
             size_t incPos;
             while((incPos = filename.find("include/", pos+1)) != std::string::npos)
               {
                  pos = incPos;
               }
             
             if(pos != 0 || filename.find("include/") == 0)
               {
                  pos += std::string("include/").size();
               }
             
             std::string module = filename.substr(pos);
             for(auto& c : module)
               {
                  c = ::toupper(c);
                  if(c == '/' || c == '.')
                    {
                       c = '_';
                    }
               }
             return module;
          }
        
        void openCpp(std::ofstream& outcpp)
          {
             outcpp << "#include \"mods/utils/Filters.hpp\"" << std::endl;
             outcpp << std::endl;
             outcpp << "namespace mods {" << std::endl;
             outcpp << "  namespace utils {" << std::endl;
          }
        
        void closeCpp(std::ofstream& outcpp)
          {
             outcpp << "  } // namespace utils" << std::endl;
             outcpp << "} // namespace mods" << std::endl;
          }
        
        void generateLowPassFilter(const LowPassParam& param, std::ofstream& out, std::ofstream& outcpp)
          {
             std::vector<Band> bands;
             double cutoffFrequency = param.cutoffFrequency / static_cast<double>(param.cutoffFrequencyDivider);
             bands.emplace_back(0.0, cutoffFrequency, 1.0, 5.0, param.sampleFrequency);
             bands.emplace_back(cutoffFrequency + 50, param.sampleFrequency/2, 0.0, -40.0, param.sampleFrequency);
             FirFilterDesigner fir(bands);
             fir.displayProgress();
             fir.optimizeFilter(param.numTaps);
             auto& taps = fir.getTaps();
             
             out << "    template<>" << std::endl;
             out << "    class LowPassFilter<" << param.cutoffFrequency << "," << param.cutoffFrequencyDivider << ',' << param.sampleFrequency << "> {" << std::endl;
             out << "     public:" << std::endl;
             out << "      LowPassFilter() = delete;" << std::endl;
             out << "      LowPassFilter(const LowPassFilter&) = delete;" << std::endl;
             out << "      LowPassFilter(LowPassFilter&&) = delete;" << std::endl;
             out << "      LowPassFilter& operator=(const LowPassFilter&) = delete;" << std::endl;
             out << "      LowPassFilter& operator=(LowPassFilter&&) = delete;" << std::endl;
             out << "      ~LowPassFilter() = delete;" << std::endl;
             out << std::endl;
             out << "     static std::array<double," << taps.size() << "> taps;" << std::endl;
             
             out << "    };" << std::endl;
             out << std::endl;
             
             outcpp << "    std::array<double," << taps.size() << "> LowPassFilter<" << param.cutoffFrequency << "," << param.cutoffFrequencyDivider << "," << param.sampleFrequency << ">::taps {" << std::endl;
             for(size_t i=0; i < taps.size(); ++i)
               {
                  if(i != 0) outcpp << ',' << std::endl;
                  outcpp << "     " << taps[i];
               }
             outcpp << std::endl;
             outcpp << "    };" << std::endl;;
          }
        
        void generateFilters(const std::string& headerFilename, const std::string& cppFilename)
          {
             std::ofstream out;
             std::ofstream outcpp;
             std::string moduleName = generateModuleName(headerFilename);
             out.open(headerFilename);
             outcpp.open(cppFilename);
             
             out << "#ifndef " << moduleName << std::endl;
             out << "#define " << moduleName << std::endl;
             out << std::endl;
             out << "#include <array>" << std::endl;
             
             out << std::endl;
             out << "namespace mods {" << std::endl;
             out << "  namespace utils {" << std::endl;
             out << std::endl;
             out << "    template<int CUTOFF, int CUTOFF_DIVISOR, int SAMPLEFREQ>" << std::endl;
             out << "    class LowPassFilter {" << std::endl;
             out << "     public:" << std::endl;
             out << "      LowPassFilter() = delete;" << std::endl;
             out << "      LowPassFilter(const LowPassFilter&) = delete;" << std::endl;
             out << "      LowPassFilter(LowPassFilter&&) = delete;" << std::endl;
             out << "      LowPassFilter& operator=(const LowPassFilter&) = delete;" << std::endl;
             out << "      LowPassFilter& operator=(LowPassFilter&&) = delete;" << std::endl;
             out << "      ~LowPassFilter() = delete;" << std::endl;
             out << "    };" << std::endl;
             out << std::endl;
             
             openCpp(outcpp);
             
             for(auto lowPassParam : lowPassParams)
               {
                  generateLowPassFilter(lowPassParam, out, outcpp);
               }
             
             closeCpp(outcpp);
             outcpp.close();
             
             out << std::endl;
             out << "  } // namespace utils" << std::endl;
             out << "} // namespace mods" << std::endl;
             out << std::endl;
             
             out << "#endif //" << moduleName << std::endl;
             out.close();
          }
     } // namespace utils
} // namespace mods

int main(int argc, char** argv)
{
   mods::utils::generateFilters(argv[1], argv[2]);
   return 0;
}
