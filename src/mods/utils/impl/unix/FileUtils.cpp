
#include "mods/utils/RBufferBackend.hpp"
#include "mods/utils/impl/unix/FileUtils.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace mods
{
   namespace utils
     {
        namespace FileUtils
          {
             namespace
               {
                  class UnixMapperDeleter : public RBufferBackend::Deleter
                    {
                     public:
                       UnixMapperDeleter(int fd, void *ptr, size_t length)
                         : _fd(fd),
                         _ptr(ptr),
                         _length(length)
                           {
                           }
                       UnixMapperDeleter() = delete;
                       UnixMapperDeleter(const UnixMapperDeleter&) = delete;
                       UnixMapperDeleter(const UnixMapperDeleter&&) = delete;
                       UnixMapperDeleter& operator=(const UnixMapperDeleter&) = delete;
                       UnixMapperDeleter& operator=(const UnixMapperDeleter&&) = delete;
                       ~UnixMapperDeleter() override
                         {
                            ::munmap(_ptr, _length);
                            ::close(_fd);
                         }
                       
                     private:
                       int _fd;
                       void* _ptr;
                       size_t _length;
                    };
               } // namespace
             
             RBufferBackend::sptr mapFileToBuffer(const std::string& filename)
               {
                  int fd = modsOpen(filename.c_str(), O_RDONLY);
                  if(fd == -1)
                    {
                       return RBufferBackend::sptr();
                    }
                  size_t length = ::lseek(fd, 0, SEEK_END);
                  if(length == static_cast<size_t>(static_cast<off_t>(-1)))
                    {
                       return RBufferBackend::sptr();
                    }
                  void* ptr = ::mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
                  if(modsHasMapFailed(ptr) == FILEUTILS_TRUE)
                    {
                       return RBufferBackend::sptr();
                    }
                  
                  auto deleter = std::make_unique<UnixMapperDeleter>(fd, ptr, length);
                  return std::make_shared<RBufferBackend>(static_cast<u8*>(ptr), length, std::move(deleter));
               }
          } // namespace FileUtils
     } // namespace utils
} // namespace mods
