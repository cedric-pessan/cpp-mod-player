
#include "config.hpp"
#include "mods/utils/OpenCLManager.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>

namespace mods
{
   namespace utils
     {
#ifdef WITH_OPENCL
        std::unique_ptr<OpenCLManager::OpenCLConfig> OpenCLManager::_config;
        std::mutex OpenCLManager::_configInitMutex;
        
        auto OpenCLManager::getConfig() -> const OpenCLConfig&
          {
             if(!_config)
               {
                  std::lock_guard<std::mutex> lock(_configInitMutex);;
                  if(!_config)
                    {
                       _config = std::make_unique<OpenCLConfig>();
                    }
               }
             return *_config;
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
             if(getenv("DISABLE_OPENCL") != nullptr)
               {
                  _enabled = false;
                  return;
               }
             
             std::vector<cl::Device> bestPlatformDevices;
             u64 bestPlatformPower = 0;
             
             std::vector<cl::Platform> platforms;
             cl::Platform::get(&platforms);
             for(auto& p : platforms)
               {
                  u64 power = 0;
                  bool gpuPlatform = false;
                  std::vector<cl::Device> usableDevices;
                  
                  std::vector<cl::Device> devices;
                  p.getDevices(CL_DEVICE_TYPE_ALL, &devices);
                  for(auto& d : devices)
                    {
                       auto versionString = d.getInfo<CL_DEVICE_VERSION>();
                       auto version = parseVersion(versionString);
                       
                       // check the device is at lease OpenCL 1.2
                       if(!version.has_value() ||
                          version->first < 1 ||
                          (version->first == 1 && version->second < 2))
                         {
                            continue;
                         }
                       
                       // check the device support operations on double floating point numbers.
                       if(d.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>() == 0 ||
                          d.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE>() == 0)
                         {
                            continue;
                         }
                       
                       auto type = d.getInfo<CL_DEVICE_TYPE>();
                       if(type == CL_DEVICE_TYPE_GPU) 
                         {
                            _enabled = true;
                            gpuPlatform = true;
                         }
                       
                       usableDevices.push_back(d);
                       power += d.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() * d.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>();
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
                  return mods::optional<std::pair<int, int>>();
               }
             
             // remove OpenCL header from string
             std::string v = versionString.substr(openCLString.size());
             auto nextSpace = v.find(' ');
             if(nextSpace != std::string::npos)
               {
                  v = v.substr(0, nextSpace);
               }
             
             // find version separator
             auto minorMajorSep = v.find('.');
             if(minorMajorSep == std::string::npos)
               {
                  return mods::optional<std::pair<int, int>>();
               }
             
             // check there is exactly 1 version separator
             if(minorMajorSep == v.size()-1 ||
                v.find('.', minorMajorSep+1) != std::string::npos)
               {
                  return mods::optional<std::pair<int, int>>();
               }
             
             std::string majorString = v.substr(0, minorMajorSep);
             std::string minorString = v.substr(minorMajorSep+1);
             
             if(!std::all_of(majorString.begin(), majorString.end(), ::isdigit) ||
                !std::all_of(minorString.begin(), minorString.end(), ::isdigit))
               {
                  return mods::optional<std::pair<int, int>>();
               }
             
             int major = ::atoi(majorString.c_str());
             int minor = ::atoi(minorString.c_str());
             
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
