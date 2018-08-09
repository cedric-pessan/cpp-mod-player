#ifndef _IMPL_FILEUTILS_HPP_
#define _IMPL_FILEUTILS_HPP_

namespace mods
{
   namespace utils
     {
        namespace FileUtils
          {
             Buffer::sptr mapFileToBuffer(const std::string& filename);
          }
     }
}

#endif // _IMPL_FILEUTILS_HPP_
