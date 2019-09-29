#ifndef MODS_UTILS_FILEUTILS_HPP
#define MODS_UTILS_FILEUTILS_HPP

#include "mods/utils/RBuffer.hpp"

namespace mods
{
   namespace utils 
     {
        namespace FileUtils
          {
             const RBuffer<u8> mapFile(const std::string& filename);
             
             RBufferBackend::sptr readFileToBuffer(const std::string& filename);
          } // namespace FileUtils
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_FILEUTILS_HPP
