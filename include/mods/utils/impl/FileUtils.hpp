#ifndef MODS_UTILS_IMPL_FILEUTILS_HPP
#define MODS_UTILS_IMPL_FILEUTILS_HPP

#include "mods/utils/RBufferBackend.hpp"

namespace mods
{
   namespace utils
     {
        namespace FileUtils
          {
             RBufferBackend::sptr mapFileToBuffer(const std::string& filename);
          } // namespace FileUtils
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_IMPL_FILEUTILS_HPP
