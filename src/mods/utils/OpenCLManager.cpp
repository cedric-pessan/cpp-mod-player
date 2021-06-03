
#include "config.hpp"
#include "mods/utils/OpenCLManager.hpp"

#include <cstdlib>

namespace mods
{
   namespace utils
     {
#ifdef WITH_OPENCL
        auto OpenCLManager::isEnabled() -> bool
          {
             return getenv("DISABLE_OPENCL") == nullptr;
          }
#else // WITH_OPENCL
        auto OpenCLManager::isEnabled() -> bool
          {
             return false;
          }
#endif // WITH_OPENCL
     } // namespace utils
} // namespace mods