
#include "mods/utils/impl/FileUtils.hpp"
#include "mods/utils/FileUtils.hpp"
#include "mods/utils/RBuffer.hpp"

#include <fstream>
#include <vector>

namespace mods
{
   namespace utils
     {
        namespace FileUtils
          {
             auto mapFile(const std::string& filename) -> RBuffer<u8>
               {
                  auto buffer = mapFileToBuffer(filename);
                  if(!buffer)
                    {
                       buffer = readFileToBuffer(filename);
                    }
                  return RBuffer<u8>(std::move(buffer));
               }
             
             namespace
               {
                  class FileReaderDeleter : public RBufferBackend::Deleter
                    {
                     public:
                       explicit FileReaderDeleter(std::vector<u8>&& v)
                         : _v(std::move(v))
                           {
                           }
                       ~FileReaderDeleter() override = default;
                       
                       FileReaderDeleter() = delete;
                       FileReaderDeleter(const FileReaderDeleter&) = delete;
                       FileReaderDeleter(FileReaderDeleter&&) = delete;
                       auto operator=(const FileReaderDeleter&) -> FileReaderDeleter& = delete;
                       auto operator=(FileReaderDeleter&&) -> FileReaderDeleter& = delete;
                       
                     private:
                       std::vector<u8> _v;
                    };
               } // namespace
             
             auto readFileToBuffer(const std::string& filename) -> RBufferBackend::ptr
               {
                  std::ifstream file(filename, std::ios::binary | std::ios::ate);
                  std::streamsize size = file.tellg();
                  file.seekg(0, std::ios::beg);
                  
                  std::vector<u8> v(size);
                  if(file.read(static_cast<char*>(static_cast<void*>(v.data())), size))
                    {
                       u8* ptr = v.data();
                       size_t length = v.size();
                       auto deleter = std::make_unique<FileReaderDeleter>(std::move(v));
                       return std::make_unique<RBufferBackend>(ptr, length, std::move(deleter));
                    }
                  return RBufferBackend::ptr();
               }
          } // namespace FileUtils
     } // namespace utils
} // namespace mods
