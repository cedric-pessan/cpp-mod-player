#ifndef _FILEUTILS_HPP_
#define _FILEUTILS_HPP_

namespace mods
{
   namespace utils 
     {
        namespace FileUtils
          {
             RBuffer<u8> mapFile(const std::string& filename);
             
             Buffer::sptr readFileToBuffer(const std::string& filename);
          }
     }
}

#endif // _FILEUTILS_HPP_
