#ifndef MODS_UTILS_OPENCLMANAGER_HPP
#define MODS_UTILS_OPENCLMANAGER_HPP

#include "config.hpp"
#include "mods/utils/optional.hpp"

#include <memory>
#include <mutex>

#ifdef WITH_OPENCL
# define CL_TARGET_OPENCL_VERSION 120      // NOLINT(cppcoreguidelines-macro-usage)
# define CL_HPP_TARGET_OPENCL_VERSION 120  // NOLINT(cppcoreguidelines-macro-usage)
# define CL_HPP_MINIMUM_OPENCL_VERSION 120 // NOLINT(cppcoreguidelines-macro-usage)
# define CL_HPP_ENABLE_EXCEPTIONS
# include <CL/opencl.hpp>
#endif // WITH_OPENCL

namespace mods
{
   namespace utils
     {
        class OpenCLManager
          {
           public:
             OpenCLManager() = delete;
             OpenCLManager(const OpenCLManager&) = delete;
             OpenCLManager(OpenCLManager&&) = delete;
             auto operator=(const OpenCLManager&) -> OpenCLManager& = delete;
             auto operator=(OpenCLManager&&) -> OpenCLManager& = delete;
             ~OpenCLManager() = delete;
             
             static auto isEnabled() -> bool;
             
#ifdef WITH_OPENCL
             static auto getContext() -> cl::Context;
             
           private:
             class OpenCLConfig
               {
                public:
                  OpenCLConfig();
                  OpenCLConfig(const OpenCLConfig&) = delete;
                  OpenCLConfig(OpenCLConfig&&) = delete;
                  auto operator=(const OpenCLConfig&) -> OpenCLConfig& = delete;
                  auto operator=(OpenCLConfig&&) -> OpenCLConfig& = delete;
                  ~OpenCLConfig() = default;
                  
                  auto isEnabled() const -> bool;
                  auto getContext() const -> cl::Context;
                  
                private:
                  static auto parseVersion(const std::string& versionString) -> mods::optional<std::pair<int, int>>;
                  
                  bool _enabled = false;
                  cl::Context _context;
               };
             
             static auto getConfig() -> const OpenCLConfig&;
#endif // WITH_OPENCL
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_OPENCLMANAGER_HPP
