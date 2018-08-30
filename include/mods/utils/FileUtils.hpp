#ifndef MODS_UTILS_FILEUTILS_HPP
#define MODS_UTILS_FILEUTILS_HPP

namespace mods
{
   namespace utils 
     {
        namespace FileUtils
          {
             RBuffer<u8> mapFile(const std::string& filename);
             
             Buffer::sptr readFileToBuffer(const std::string& filename);
          } // namespace FileUtils
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_FILEUTILS_HPP
