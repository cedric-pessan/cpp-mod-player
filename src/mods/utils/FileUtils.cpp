
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
             RBuffer<u8> mapFile(const std::string& filename)
               {
                  auto buffer = mapFileToBuffer(filename);
                  if(!buffer)
                    {
                       buffer = readFileToBuffer(filename);
                    }
                  return RBuffer<u8>(buffer);
               }
             
             namespace
               {
                  class FileReaderDeleter : public Buffer::Deleter
                    {
                     public:
                       explicit FileReaderDeleter(std::vector<u8>&& v)
                         : _v(std::move(v))
                           {
                           }
                       ~FileReaderDeleter() override = default;
                       
                       FileReaderDeleter() = delete;
                       FileReaderDeleter(const FileReaderDeleter&) = delete;
                       FileReaderDeleter(const FileReaderDeleter&&) = delete;
                       FileReaderDeleter& operator=(const FileReaderDeleter&) = delete;
                       FileReaderDeleter& operator=(const FileReaderDeleter&&) = delete;
                       
                     private:
                       std::vector<u8> _v;
                    };
               } // namespace
             
             Buffer::sptr readFileToBuffer(const std::string& filename)
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
                       return std::make_shared<Buffer>(ptr, length, std::move(deleter));
                    }
                  return Buffer::sptr();
               }
          } // namespace FileUtils
     } // namespace utils
} // namespace mods
