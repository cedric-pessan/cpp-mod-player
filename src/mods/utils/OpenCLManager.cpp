
#include "config.hpp"
#include "mods/utils/OpenCLManager.hpp"
#include "mods/utils/optional.hpp"
#include "mods/utils/types.hpp"

#include <CL/cl.h>
#include <CL/opencl.hpp>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

namespace mods
{
   namespace utils
     {
#ifdef WITH_OPENCL
        auto OpenCLManager::getConfig() -> const OpenCLConfig&
          {
             static const OpenCLConfig config;
             return config;
          }
        
        auto OpenCLManager::isEnabled() -> bool
          {
             return getConfig().isEnabled();
          }
        
        auto OpenCLManager::getContext() -> cl::Context
          {
             return getConfig().getContext();
          }
        
        OpenCLManager::OpenCLConfig::OpenCLConfig()
          {
             if(getenv("DISABLE_OPENCL") != nullptr) // NOLINT(concurrency-mt-unsafe)
               {
                  _enabled = false;
                  return;
               }
             
             std::vector<cl::Device> bestPlatformDevices;
             u64 bestPlatformPower = 0;
             
             std::vector<cl::Platform> platforms;
             cl::Platform::get(&platforms);
             for(auto& platform : platforms)
               {
                  u64 power = 0;
                  bool gpuPlatform = false;
                  std::vector<cl::Device> usableDevices;
                  
                  std::vector<cl::Device> devices;
                  platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
                  for(auto& device : devices)
                    {
                       auto versionString = device.getInfo<CL_DEVICE_VERSION>();
                       auto version = parseVersion(versionString);
                       
                       // check the device is at lease OpenCL 1.2
                       if(!version.has_value() ||
                          version->first < 1 ||
                          (version->first == 1 && version->second < 2))
                         {
                            continue;
                         }
                       
                       // check the device support operations on double floating point numbers.
                       if(device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>() == 0 ||
                          device.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE>() == 0)
                         {
                            continue;
                         }
                       
                       auto type = device.getInfo<CL_DEVICE_TYPE>();
                       if(type == CL_DEVICE_TYPE_GPU)
                         {
                            _enabled = true;
                            gpuPlatform = true;
                         }
                       
                       usableDevices.push_back(device);
                       power += static_cast<u64>(device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>()) * static_cast<u64>(device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>());
                    }
                  
                  if(gpuPlatform && power > bestPlatformPower)
                    {
                       bestPlatformDevices = std::move(usableDevices);
                       bestPlatformPower = power;
                    }
               }
             
             _context = cl::Context(bestPlatformDevices);
          }
        
        auto OpenCLManager::OpenCLConfig::isEnabled() const -> bool
          {
             return _enabled;
          }
        
        auto OpenCLManager::OpenCLConfig::getContext() const -> cl::Context
          {
             return _context;
          }
        
        auto OpenCLManager::OpenCLConfig::parseVersion(const std::string& versionString) -> mods::optional<std::pair<int, int>>
          {
             static const std::string openCLString = "OpenCL ";
             
             // format : OpenCL M.m implementation_specific_string
             if(versionString.size() <= openCLString.size() ||
                versionString.rfind(openCLString, 0) != 0)
               {
                  return {};
               }
             
             // remove OpenCL header from string
             std::string strV = versionString.substr(openCLString.size());
             auto nextSpace = strV.find(' ');
             if(nextSpace != std::string::npos)
               {
                  strV = strV.substr(0, nextSpace);
               }
             
             // find version separator
             auto minorMajorSep = strV.find('.');
             if(minorMajorSep == std::string::npos)
               {
                  return {};
               }
             
             // check there is exactly 1 version separator
             if(minorMajorSep == strV.size()-1 ||
                strV.find('.', minorMajorSep+1) != std::string::npos)
               {
                  return {};
               }
             
             std::string majorString = strV.substr(0, minorMajorSep);
             std::string minorString = strV.substr(minorMajorSep+1);
             
             if(!std::all_of(majorString.begin(), majorString.end(), ::isdigit) ||
                !std::all_of(minorString.begin(), minorString.end(), ::isdigit))
               {
                  return {};
               }
             
             const u64 major = ::strtol(majorString.c_str(), nullptr, 0);
             const u64 minor = ::strtol(minorString.c_str(), nullptr, 0);
             
             return mods::optional<std::pair<int, int>>(major, minor);
          }
#else // WITH_OPENCL
        auto OpenCLManager::isEnabled() -> bool
          {
             return false;
          }
#endif // WITH_OPENCL
     } // namespace utils
} // namespace mods
