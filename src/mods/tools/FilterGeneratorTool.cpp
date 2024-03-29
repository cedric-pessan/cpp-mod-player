
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "mods/utils/ConstFraction.hpp"
#include "mods/utils/FirFilterDesigner.hpp"
#include "mods/utils/RBuffer.hpp"

namespace mods
{
   namespace tools
     {
        class LowPassParam
          {
           public:
             constexpr LowPassParam(int cutoffFrequency, int cutoffFrequencyDivider, u64 sampleFrequency,
                                    int inFrequency, int outFrequency,
                                    double expectedAttenuation, double transitionWidth)
               : _cutoffFrequency(cutoffFrequency),
               _cutoffFrequencyDivider(cutoffFrequencyDivider),
               _sampleFrequency(sampleFrequency),
               _inFrequency(inFrequency),
               _outFrequency(outFrequency),
               _expectedAttenuation(expectedAttenuation),
               _transitionWidth(transitionWidth)
                 {
                 }
             
             LowPassParam() = delete;
             LowPassParam(const LowPassParam&) = default;
             LowPassParam(LowPassParam&&) = delete;
             auto operator=(const LowPassParam&) -> LowPassParam& = /*default*/delete;
             auto operator=(LowPassParam&&) -> LowPassParam& = delete;
             ~LowPassParam() = default;
             
             constexpr auto getCutoffFrequency() const -> int
               {
                  return _cutoffFrequency;
               }
             
             constexpr auto getCutoffFrequencyDivider() const -> int
               {
                  return _cutoffFrequencyDivider;
               }
             
             constexpr auto getSampleFrequency() const -> u64
               {
                  return _sampleFrequency;
               }
             
             constexpr auto getExpectedAttenuation() const -> double
               {
                  return _expectedAttenuation;
               }
             
             constexpr auto getTransitionWidth() const -> double
               {
                  return _transitionWidth;
               }
             
             auto getFilterName() const -> std::string
               {
                  std::stringstream ss;
                  ss << "f" << _inFrequency << "_to_" << _outFrequency << "_cutto_" << _cutoffFrequency << "_" << _cutoffFrequencyDivider;
                  return ss.str();
               }
             
           private:
             int _cutoffFrequency;
             int _cutoffFrequencyDivider;
             u64 _sampleFrequency;
             
             int _inFrequency;
             int _outFrequency;
             
             double _expectedAttenuation;
             double _transitionWidth;
          };
        
        auto getLowPassParams() -> const std::vector<LowPassParam>&
          {
             using mods::utils::ConstFraction;
             static constexpr std::initializer_list<LowPassParam> lowPassParamsList =
               {
                  { 22000, 2, 22000 * ConstFraction(22000,44100).reduce().getDenominator(), 22000, 44100, 40.0, 50.0 }, // 22kHz -> 44100Hz
                  { 8000,  2, 8000 *  ConstFraction(8000,44100).reduce().getDenominator(),  8000,  44100, 40.0, 50.0 }, // 8kHz -> 44100Hz
                  { 44100, 2, 48000 * ConstFraction(48000,44100).reduce().getDenominator(), 48000, 44100, 40.0, 50.0 }, // 48kHz -> 44100Hz
                  { 10000, 2, 10000 * ConstFraction(10000,44100).reduce().getDenominator(), 10000, 44100, 40.0, 50.0 }, // 10kHz -> 44100Hz
                  
                  { 44100, 2, 3546895 * ConstFraction(3546895,44100).reduce().getDenominator(), 3546895, 44100, 36.0, 308700.0 }, // amiga without led filter
                  { 3275, 1, 3546895 * ConstFraction(3546895,44100).reduce().getDenominator(), 3546895, 44100, 36.0, 308700.0 }   // amiga with led filter
               };
             static std::vector<LowPassParam> lowPassParams(lowPassParamsList);
             return lowPassParams;
          }
        
        auto generateModuleName(const std::string& filename) -> std::string
          {
             std::string unixFilename = filename;
             for(auto& c : unixFilename)
               {
                  if(c == '\\')
                    {
                       c = '/';
                    }
               }
             
             size_t pos = 0;
             size_t incPos = 0;
             while((incPos = unixFilename.find("include/", pos+1)) != std::string::npos)
               {
                  pos = incPos;
               }
             
             if(pos != 0 || unixFilename.rfind("include/", 0) == 0)
               {
                  pos += std::string("include/").size();
               }
             
             std::string module = unixFilename.substr(pos);
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
             double cutoffFrequency = param.getCutoffFrequency() / static_cast<double>(param.getCutoffFrequencyDivider());
             std::cout << "Generate " << cutoffFrequency << "Hz low pass filter on a " << param.getSampleFrequency() << "Hz sampling rate" << std::endl;
             mods::utils::FirFilterDesigner fir(param.getSampleFrequency(), cutoffFrequency, param.getExpectedAttenuation(), param.getTransitionWidth());
             const auto& taps = fir.getTaps();
             
             auto filterName = param.getFilterName();
             
             out << "    constexpr LowPassParam " << filterName << " {" << std::endl;
             out << "      " << param.getCutoffFrequency() << "," << std::endl;
             out << "      " << param.getCutoffFrequencyDivider() << "," << std::endl;
             out << "      " << param.getSampleFrequency() << std::endl;
             out << "    };" << std::endl;
             out << std::endl;
             
             out << "    template<>" << std::endl;
             out << "    class LowPassFilter<" << filterName << ".cutoffFrequency, " << filterName << ".cutoffFrequencyDivider, " << filterName << ".sampleFrequency> {" << std::endl;
             out << "     public:" << std::endl;
             out << "      LowPassFilter() = delete;" << std::endl;
             out << "      LowPassFilter(const LowPassFilter&) = delete;" << std::endl;
             out << "      LowPassFilter(LowPassFilter&&) = delete;" << std::endl;
             out << "      auto operator=(const LowPassFilter&) -> LowPassFilter& = delete;" << std::endl;
             out << "      auto operator=(LowPassFilter&&) -> LowPassFilter& = delete;" << std::endl;
             out << "      ~LowPassFilter() = delete;" << std::endl;
             out << std::endl;
             out << "      constexpr static int numberOfTaps = " << taps.size() << ";" << std::endl;
             out << "      using TapsType = mods::utils::RBuffer<double>;" << std::endl;
             out << std::endl;
             out << "      static auto getTap(size_t i) -> double;" << std::endl;
             out << "      static auto getTaps() -> const TapsType&;" << std::endl;
             out << std::endl;
             out << "     private:" << std::endl;
             out << "      static const std::array<u8, numberOfTaps * sizeof(double)> _tapsBinaryImage;" << std::endl;
             out << "    };" << std::endl;
             out << std::endl;
             
             outcpp << "    constexpr int LowPassFilter< " << param.getCutoffFrequency() << "," << param.getCutoffFrequencyDivider() << "," << param.getSampleFrequency() << ">::numberOfTaps;" << std::endl;
             outcpp << std::endl;
             
             auto deleter = std::make_unique<mods::utils::RBufferBackend::EmptyDeleter>();
             auto* ptr = static_cast<const u8*>(static_cast<const void*>(taps.data()));
             auto tapsBackend = std::make_unique<mods::utils::RBufferBackend>(ptr, taps.size() * sizeof(double), std::move(deleter));
             mods::utils::RBuffer<u8> tapsBinaryBuffer(std::move(tapsBackend));
             
             outcpp << "    const std::array<u8," << (taps.size() * sizeof(double)) << "> LowPassFilter<" << param.getCutoffFrequency() << "," << param.getCutoffFrequencyDivider() << "," << param.getSampleFrequency() << ">::_tapsBinaryImage {" << std::endl;
             for(size_t i=0; i < taps.size() * sizeof(double); ++i)
               {
                  if(i != 0) {
                     outcpp << ',' << std::endl;
                  }
                  outcpp << "     " << static_cast<u32>(tapsBinaryBuffer[i]);
               }
             outcpp << std::endl;
             outcpp << "    };" << std::endl;
             
             outcpp << std::endl;
             outcpp << "    auto LowPassFilter<" << param.getCutoffFrequency() << "," << param.getCutoffFrequencyDivider() << "," << param.getSampleFrequency() << ">::getTap(size_t i) -> double {" << std::endl;
             outcpp << "      return getTaps()[i];" << std::endl;
             outcpp << "    }" << std::endl;
             outcpp << std::endl;
             
             outcpp << "    auto LowPassFilter<" << param.getCutoffFrequency() << "," << param.getCutoffFrequencyDivider() << "," << param.getSampleFrequency() << ">::getTaps() -> const TapsType& {" << std::endl;
             outcpp << "      static auto deleter = std::make_unique<mods::utils::RBufferBackend::EmptyDeleter>();" << std::endl;
             outcpp << "      static auto backend = std::make_unique<mods::utils::RBufferBackend>(_tapsBinaryImage.data(), _tapsBinaryImage.size(), std::move(deleter));" << std::endl;
             outcpp << "      static mods::utils::RBuffer<u8> buffer(std::move(backend));" << std::endl;
             outcpp << "      static auto doubleView = buffer.slice<double>(0, numberOfTaps);" << std::endl;
             outcpp << "      return doubleView;" << std::endl;
             outcpp << "    }" << std::endl;
             outcpp << std::endl;
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
             out << "#include \"mods/utils/RBuffer.hpp\"" << std::endl;
             out << "#include \"mods/utils/types.hpp\"" << std::endl;
             out << std::endl;
             out << "#include <array>" << std::endl;
             
             out << std::endl;
             out << "namespace mods {" << std::endl;
             out << "  namespace utils {" << std::endl;
             
             out << std::endl;
             out << "    struct LowPassParam {" << std::endl;
             out << "      int cutoffFrequency;" << std::endl;
             out << "      int cutoffFrequencyDivider;" << std::endl;
             out << "      u64 sampleFrequency;" << std::endl;
             out << "    };" << std::endl;
             
             out << std::endl;
             out << "    template<int CUTOFF, int CUTOFF_DIVISOR, u64 SAMPLEFREQ>" << std::endl;
             out << "    class LowPassFilter {" << std::endl;
             out << "     public:" << std::endl;
             out << "      LowPassFilter() = delete;" << std::endl;
             out << "      LowPassFilter(const LowPassFilter&) = delete;" << std::endl;
             out << "      LowPassFilter(LowPassFilter&&) = delete;" << std::endl;
             out << "      auto operator=(const LowPassFilter&) -> LowPassFilter& = delete;" << std::endl;
             out << "      auto operator=(LowPassFilter&&) -> LowPassFilter& = delete;" << std::endl;
             out << "      ~LowPassFilter() = delete;" << std::endl;
             out << "    };" << std::endl;
             out << std::endl;
             
             openCpp(outcpp);
             
             for(const auto& lowPassParam : getLowPassParams())
               {
                  generateLowPassFilter(lowPassParam, out, outcpp);
               }
             
             closeCpp(outcpp);
             outcpp.close();
             
             out << std::endl;
             out << "  } // namespace utils" << std::endl;
             out << "} // namespace mods" << std::endl;
             out << std::endl;
             
             out << "#endif // " << moduleName << std::endl;
             out.close();
          }
     } // namespace tools
} // namespace mods

auto main(int argc, char** argv) -> int
{
   if(argc < 3)
     {
        return -1;
     }
   
   auto deleter = std::make_unique<mods::utils::RBufferBackend::EmptyDeleter>();
   auto* argvp = static_cast<u8*>(static_cast<void*>(argv));
   auto argBuffer = std::make_unique<mods::utils::RBufferBackend>(argvp, argc * sizeof(char*), std::move(deleter));
   const mods::utils::RBuffer<char*> args(std::move(argBuffer));
   
   mods::tools::generateFilters(args[1], args[2]);
   return 0;
}
