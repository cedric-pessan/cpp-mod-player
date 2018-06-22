
#include "../../Buffer.hpp"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

namespace mods
{
   namespace FileUtils
     {
        namespace
          {
             class UnixMapperDeleter : public Buffer::Deleter
               {
                public:
                  UnixMapperDeleter(int fd, void *ptr, size_t length)
                    : _fd(fd),
                    _ptr(ptr),
                    _length(length)
                    {
                    }
                  virtual ~UnixMapperDeleter()
                    {
                       ::munmap(_ptr, _length);
                       ::close(_fd);
                    }
                  
                private:
                  UnixMapperDeleter() = delete;
                  UnixMapperDeleter(const UnixMapperDeleter&) = delete;
                  UnixMapperDeleter& operator=(const UnixMapperDeleter&) = delete;
                  
                  int _fd;
                  void* _ptr;
                  size_t _length;
               };
          }
        
        Buffer::sptr mapFileToBuffer(const std::string& filename)
          {
             int fd = ::open(filename.c_str(), O_RDONLY);
             size_t length = ::lseek(fd, 0, SEEK_END);
             void* ptr = ::mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
             
             auto deleter = std::make_unique<UnixMapperDeleter>(fd, ptr, length);
             return std::make_shared<Buffer>(nullptr, 0, std::move(deleter));
          }
     }
}
