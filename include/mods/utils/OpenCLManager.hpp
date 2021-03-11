#ifndef MODS_UTILS_OPENCLMANAGER_HPP
#define MODS_UTILS_OPENCLMANAGER_HPP

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
             
             auto static isEnabled() -> bool;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_OPENCLMANAGER_HPP
