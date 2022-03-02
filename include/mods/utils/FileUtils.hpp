#ifndef MODS_UTILS_FILEUTILS_HPP
#define MODS_UTILS_FILEUTILS_HPP

#include "mods/utils/RBuffer.hpp"

namespace mods
{
   namespace utils 
     {
        namespace FileUtils
          {
             auto mapFile(const std::string& filename) -> RBuffer<u8>;
             
             auto readFileToBuffer(const std::string& filename) -> RBufferBackend::ptr;
          } // namespace FileUtils
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_FILEUTILS_HPP
