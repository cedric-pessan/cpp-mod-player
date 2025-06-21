
#include "mods/utils/RBufferBackend.hpp"
#include "mods/utils/impl/FileUtils.hpp"
#include "mods/utils/impl/unix/FileUtils.h"
#include "mods/utils/types.hpp"

#include <cstddef>
#include <cstdio>
#include <fcntl.h>
#include <memory>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>

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
                       UnixMapperDeleter(int fileD, void *ptr, size_t length)
                         : _fd(fileD),
                         _ptr(ptr),
                         _length(length)
                           {
                           }
                       UnixMapperDeleter() = delete;
                       UnixMapperDeleter(const UnixMapperDeleter&) = delete;
                       UnixMapperDeleter(UnixMapperDeleter&&) = delete;
                       auto operator=(const UnixMapperDeleter&) -> UnixMapperDeleter& = delete;
                       auto operator=(UnixMapperDeleter&&) -> UnixMapperDeleter& = delete;
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
             
             auto mapFileToBuffer(const std::string& filename) -> RBufferBackend::ptr
               {
                  const int fileDescriptor = modsOpen(filename.c_str(), O_RDONLY);
                  if(fileDescriptor == -1)
                    {
                       return {};
                    }
                  const size_t length = ::lseek(fileDescriptor, 0, SEEK_END);
                  if(length == static_cast<size_t>(1))
                    {
                       return {};
                    }
                  void* ptr = ::mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fileDescriptor, 0);
                  if(modsHasMapFailed(ptr) == FILEUTILS_TRUE)
                    {
                       return {};
                    }
                  
                  auto deleter = std::make_unique<UnixMapperDeleter>(fileDescriptor, ptr, length);
                  return std::make_unique<RBufferBackend>(static_cast<u8*>(ptr), length, std::move(deleter));
               }
          } // namespace FileUtils
     } // namespace utils
} // namespace mods
