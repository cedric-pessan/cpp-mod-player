
#include "config.hpp"
#include "mods/utils/OpenCLManager.hpp"

#include <cstdlib>

namespace mods
{
   namespace utils
     {
#ifdef WITH_OPENCL
        bool OpenCLManager::isEnabled()
          {
             if(getenv("DISABLE_OPENCL") != nullptr)
               {
                  return false;
               }
             return true;
          }
#else // WITH_OPENCL
        bool OpenCLManager::isEnabled()
          {
             return false;
          }
#endif // WITH_OPENCL
     } // namespace utils
} // namespace mods