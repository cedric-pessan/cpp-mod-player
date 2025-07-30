
#include <cctype>
#include <cstddef>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "mods/utils/ConstFraction.hpp"
#include "mods/utils/FirFilterDesigner.hpp"
#include "mods/utils/RBuffer.hpp"
#include "mods/utils/RBufferBackend.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace tools
     {
        class LowPassParam
          {
           private:
             constexpr static int targetFrequency = 44100;
             
           public:
             struct Params
               {
                  int cutoffFrequency;
                  int cutoffFrequencyDivider;
                  int inFrequency;
                  int outFrequency;
                  double expectedAttenuation;
                  double transitionWidth;
               };
             
             constexpr explicit LowPassParam(Params params)
               : _cutoffFrequency(params.cutoffFrequency),
               _cutoffFrequencyDivider(params.cutoffFrequencyDivider),
               _sampleFrequency(params.inFrequency * mods::utils::ConstFraction(static_cast<mods::utils::ConstFraction::Numerator>(params.inFrequency),
                                                                                static_cast<mods::utils::ConstFraction::Denominator>(targetFrequency)).reduce().getDenominator()),
               _inFrequency(params.inFrequency),
               _outFrequency(params.outFrequency),
               _expectedAttenuation(params.expectedAttenuation),
               _transitionWidth(params.transitionWidth)
                 {
                 }
             
             LowPassParam() = delete;
             LowPassParam(const LowPassParam&) = default;
             LowPassParam(LowPassParam&&) = default;
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
                  std::stringstream sstream;
                  sstream << "f" << _inFrequency << "_to_" << _outFrequency << "_cutto_" << _cutoffFrequency << "_" << _cutoffFrequencyDivider;
                  return sstream.str();
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
        
        namespace
          {
             auto getLowPassParams() -> const std::vector<LowPassParam>&
               {
                  static constexpr std::initializer_list<LowPassParam> lowPassParamsList =
                    {
                       LowPassParam(LowPassParam::Params{22000, 2, 22000,   44100, 40.0, 50.0}),     // 22kHz -> 44100Hz
                       LowPassParam(LowPassParam::Params{8000,  2, 8000,    44100, 40.0, 50.0}),     // 8kHz  -> 44100Hz
                       LowPassParam(LowPassParam::Params{44100, 2, 48000,   44100, 40.0, 50.0}),     // 48kHz -> 44100Hz
                       LowPassParam(LowPassParam::Params{10000, 2, 10000,   44100, 40.0, 50.0}),     // 10kHz -> 44100Hz
                       
                       LowPassParam(LowPassParam::Params{44100, 2, 3546895, 44100, 36.0, 308700.0}), // amiga without led filter
                       LowPassParam(LowPassParam::Params{3275,  1, 3546895, 44100, 36.0, 308700.0})  // amiga with led filter
                    };
                  static const std::vector<LowPassParam> lowPassParams(lowPassParamsList);
                  return lowPassParams;
               }
             
             auto generateModuleName(const std::string& filename) -> std::string
               {
                  std::string unixFilename = filename;
                  for(auto& fChar : unixFilename)
                    {
                       if(fChar == '\\')
                         {
                            fChar = '/';
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
                  for(auto& mChar : module)
                    {
                       mChar = static_cast<char>(::toupper(mChar));
                       if(mChar == '/' || mChar == '.')
                         {
                            mChar = '_';
                         }
                    }
                  return module;
               }
             
             void openCpp(std::ofstream& outcpp)
               {
                  outcpp << "#include \"mods/utils/Filters.hpp\"" << '\n';
                  outcpp << "#include \"mods/utils/RBufferBackend.hpp\"" << '\n';
                  outcpp << "#include \"mods/utils/types.hpp\"" << '\n';
                  outcpp << '\n';
                  outcpp << "#include <array>" << '\n';
                  outcpp << "#include <cstddef>" << '\n';
                  outcpp << "#include <memory>" << '\n';
                  outcpp << "#include <utility>" << '\n';
                  outcpp << '\n';
                  outcpp << "namespace mods {" << '\n';
                  outcpp << "  namespace utils {" << '\n';
               }
             
             void closeCpp(std::ofstream& outcpp)
               {
                  outcpp << "  } // namespace utils" << '\n';
                  outcpp << "} // namespace mods" << '\n';
               }
             
             void generateLowPassFilter(const LowPassParam& param, std::ofstream& out, std::ofstream& outcpp, std::unordered_set<int>* definedCutoff, std::unordered_set<u64>* definedTmpFreq)
               {
                  using mods::utils::FirFilterDesigner;
                  
                  const double cutoffFrequency = param.getCutoffFrequency() / static_cast<double>(param.getCutoffFrequencyDivider());
                  std::cout << "Generate " << cutoffFrequency << "Hz low pass filter on a " << param.getSampleFrequency() << "Hz sampling rate" << '\n';
                  const FirFilterDesigner fir(FirFilterDesigner::Params{param.getSampleFrequency(), cutoffFrequency, param.getExpectedAttenuation(), param.getTransitionWidth()});
                  const auto& taps = fir.getTaps();
                  
                  auto filterName = param.getFilterName();
                  
                  out << "    constexpr LowPassParam " << filterName << " {" << '\n';
                  out << "      " << param.getCutoffFrequency() << "," << '\n';
                  out << "      " << param.getCutoffFrequencyDivider() << "," << '\n';
                  out << "      " << param.getSampleFrequency() << '\n';
                  out << "    };" << '\n';
                  out << '\n';
                  
                  out << "    template<>" << '\n';
                  out << "    class LowPassFilter<" << filterName << ".cutoffFrequency, " << filterName << ".cutoffFrequencyDivider, " << filterName << ".sampleFrequency> {" << '\n';
                  out << "     public:" << '\n';
                  out << "      LowPassFilter() = delete;" << '\n';
                  out << "      LowPassFilter(const LowPassFilter&) = delete;" << '\n';
                  out << "      LowPassFilter(LowPassFilter&&) = delete;" << '\n';
                  out << "      auto operator=(const LowPassFilter&) -> LowPassFilter& = delete;" << '\n';
                  out << "      auto operator=(LowPassFilter&&) -> LowPassFilter& = delete;" << '\n';
                  out << "      ~LowPassFilter() = delete;" << '\n';
                  out << '\n';
                  out << "      constexpr static int numberOfTaps = " << taps.size() << ';' << '\n';
                  out << "      using TapsType = mods::utils::RBuffer<double>;" << '\n';
                  out << '\n';
                  out << "      static auto getTap(size_t i) -> double;" << '\n';
                  out << "      static auto getTaps() -> const TapsType&;" << '\n';
                  out << '\n';
                  out << "     private:" << '\n';
                  out << "      static const std::array<u8, numberOfTaps * sizeof(double)> _tapsBinaryImage;" << '\n';
                  out << "    };" << '\n';
                  out << '\n';
                  
                  outcpp << "    constexpr int LowPassFilter< " << param.getCutoffFrequency() << "," << param.getCutoffFrequencyDivider() << "," << param.getSampleFrequency() << ">::numberOfTaps;" << '\n';
                  outcpp << '\n';
                  
                  auto deleter = std::make_unique<mods::utils::RBufferBackend::EmptyDeleter>();
                  auto tapsBackend = std::make_unique<mods::utils::RBufferBackend>(taps.data(), taps.size(), std::move(deleter));
                  const mods::utils::RBuffer<u8> tapsBinaryBuffer(std::move(tapsBackend));
                  
                  outcpp << "    const std::array<u8," << (taps.size() * sizeof(double)) << "> LowPassFilter<" << param.getCutoffFrequency() << "," << param.getCutoffFrequencyDivider() << "," << param.getSampleFrequency() << ">::_tapsBinaryImage {" << '\n';
                  for(size_t i=0; i < taps.size() * sizeof(double); ++i)
                    {
                       if(i != 0) {
                          outcpp << ',' << '\n';
                       }
                       outcpp << "     " << static_cast<u32>(tapsBinaryBuffer[i]);
                    }
                  outcpp << '\n';
                  outcpp << "    };" << '\n';
                  
                  if(definedCutoff->find(param.getCutoffFrequency()) == definedCutoff->end())
                    {
                       outcpp << '\n';
                       outcpp << "  namespace {" << '\n';
                       outcpp << "    constexpr int cutOff_" << param.getCutoffFrequency() << " = " << param.getCutoffFrequency() << ';' << '\n';
                       outcpp << "  } // namespace" << '\n';
                       outcpp << '\n';
                       definedCutoff->insert(param.getCutoffFrequency());
                    }
                  if(definedTmpFreq->find(param.getSampleFrequency()) == definedTmpFreq->end())
                    {
                       outcpp << '\n';
                       outcpp << "  namespace {" << '\n';
                       outcpp << "    constexpr u64 tmpFreq_" << param.getSampleFrequency() << " = " << param.getSampleFrequency() << ";" << '\n';
                       outcpp << "  } // namespace" << '\n';
                       outcpp << '\n';
                       definedTmpFreq->insert(param.getSampleFrequency());
                    }
                  
                  outcpp << '\n';
                  outcpp << "    auto LowPassFilter<cutOff_" << param.getCutoffFrequency() << "," << param.getCutoffFrequencyDivider() << ", tmpFreq_" << param.getSampleFrequency() << ">::getTap(size_t idx) -> double {" << '\n';
                  outcpp << "      return getTaps()[idx];" << '\n';
                  outcpp << "    }" << '\n';
                  outcpp << '\n';
                  
                  outcpp << "    auto LowPassFilter<cutOff_" << param.getCutoffFrequency() << "," << param.getCutoffFrequencyDivider() << ", tmpFreq_" << param.getSampleFrequency() << ">::getTaps() -> const TapsType& {" << '\n';
                  outcpp << "      static auto deleter = std::make_unique<mods::utils::RBufferBackend::EmptyDeleter>();" << '\n';
                  outcpp << "      static auto backend = std::make_unique<mods::utils::RBufferBackend>(_tapsBinaryImage.data(), _tapsBinaryImage.size(), std::move(deleter));" << '\n';
                  outcpp << "      static const mods::utils::RBuffer<u8> buffer(std::move(backend));" << '\n';
                  outcpp << "      static auto doubleView = buffer.slice<double>(0, numberOfTaps);" << '\n';
                  outcpp << "      return doubleView;" << '\n';
                  outcpp << "    }" << '\n';
                  outcpp << '\n';
               }
             
             void generateFilters(const std::string& headerFilename, const std::string& cppFilename)
               {
                  std::unordered_set<int> definedCutoff;
                  std::unordered_set<u64> definedTmpFreq;
                  std::ofstream out;
                  std::ofstream outcpp;
                  const std::string moduleName = generateModuleName(headerFilename);
                  out.open(headerFilename);
                  outcpp.open(cppFilename);
                  
                  out << "#ifndef " << moduleName << '\n';
                  out << "#define " << moduleName << '\n';
                  out << '\n';
                  out << "#include \"mods/utils/RBuffer.hpp\"" << '\n';
                  out << "#include \"mods/utils/types.hpp\"" << '\n';
                  out << '\n';
                  out << "#include <array>" << '\n';
                  
                  out << '\n';
                  out << "namespace mods {" << '\n';
                  out << "  namespace utils {" << '\n';
                  
                  out << '\n';
                  out << "    struct LowPassParam {" << '\n';
                  out << "      int cutoffFrequency;" << '\n';
                  out << "      int cutoffFrequencyDivider;" << '\n';
                  out << "      u64 sampleFrequency;" << '\n';
                  out << "    };" << '\n';
                  
                  out << '\n';
                  out << "    template<int CUTOFF, int CUTOFF_DIVISOR, u64 SAMPLEFREQ>" << '\n';
                  out << "    class LowPassFilter {" << '\n';
                  out << "     public:" << '\n';
                  out << "      LowPassFilter() = delete;" << '\n';
                  out << "      LowPassFilter(const LowPassFilter&) = delete;" << '\n';
                  out << "      LowPassFilter(LowPassFilter&&) = delete;" << '\n';
                  out << "      auto operator=(const LowPassFilter&) -> LowPassFilter& = delete;" << '\n';
                  out << "      auto operator=(LowPassFilter&&) -> LowPassFilter& = delete;" << '\n';
                  out << "      ~LowPassFilter() = delete;" << '\n';
                  out << "    };" << '\n';
                  out << '\n';
                  
                  openCpp(outcpp);
                  
                  for(const auto& lowPassParam : getLowPassParams())
                    {
                       generateLowPassFilter(lowPassParam, out, outcpp, &definedCutoff, &definedTmpFreq);
                    }
                  
                  closeCpp(outcpp);
                  outcpp.close();
                  
                  out << '\n';
                  out << "  } // namespace utils" << '\n';
                  out << "} // namespace mods" << '\n';
                  out << '\n';
                  
                  out << "#endif // " << moduleName << '\n';
                  out.close();
               }
          } // namespace
     } // namespace tools
} // namespace mods

auto main(int argc, char** argv) -> int
{
   if(argc < 3)
     {
        return -1;
     }
   
   auto deleter = std::make_unique<mods::utils::RBufferBackend::EmptyDeleter>();
   auto argBuffer = std::make_unique<mods::utils::RBufferBackend>(argv, argc, std::move(deleter));
   const mods::utils::RBuffer<char*> args(std::move(argBuffer));
   
   mods::tools::generateFilters(args[1], args[2]);
   return 0;
}
