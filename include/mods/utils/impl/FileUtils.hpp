#ifndef MODS_UTILS_IMPL_FILEUTILS_HPP
#define MODS_UTILS_IMPL_FILEUTILS_HPP

#include "mods/utils/RBufferBackend.hpp"

namespace mods
{
   namespace utils
     {
        namespace FileUtils
          {
             auto mapFileToBuffer(const std::string& filename) -> RBufferBackend::sptr;
          } // namespace FileUtils
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_IMPL_FILEUTILS_HPP
