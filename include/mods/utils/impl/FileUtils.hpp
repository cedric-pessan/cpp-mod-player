#ifndef MODS_UTILS_IMPL_FILEUTILS_HPP
#define MODS_UTILS_IMPL_FILEUTILS_HPP

#include "mods/utils/BufferBackend.hpp"

namespace mods
{
   namespace utils
     {
        namespace FileUtils
          {
             BufferBackend::sptr mapFileToBuffer(const std::string& filename);
          } // namespace FileUtils
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_IMPL_FILEUTILS_HPP
