
#include "RBuffer.hpp"
#include "FileUtils.hpp"
#include "impl/FileUtils.hpp"

#include <vector>
#include <fstream>

namespace mods
{
   namespace FileUtils
     {
        RBuffer<u8> mapFile(const std::string& filename)
          {
             auto buffer = mapFileToBuffer(filename);
             if(!buffer)
               buffer = readFileToBuffer(filename);
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
                  virtual ~FileReaderDeleter()
                    {
                    }
                  
                private:
                  FileReaderDeleter() = delete;
                  FileReaderDeleter(const FileReaderDeleter&) = delete;
                  FileReaderDeleter& operator=(const FileReaderDeleter&) = delete;
                  
                  std::vector<u8> _v;
               };
          }
        
        Buffer::sptr readFileToBuffer(const std::string& filename)
          {
             std::ifstream file(filename, std::ios::binary | std::ios::ate);
             std::streamsize size = file.tellg();
             file.seekg(0, std::ios::beg);
             
             std::vector<u8> v(size);
             if(file.read(reinterpret_cast<char*>(v.data()), size))
               {
                  u8* ptr = v.data();
                  size_t length = v.size();
                  auto deleter = std::make_unique<FileReaderDeleter>(std::move(v));
                  return std::make_shared<Buffer>(ptr, length, std::move(deleter));
               }
             else
               return Buffer::sptr();
          }
     }
}
