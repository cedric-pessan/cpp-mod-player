
#include "mods/utils/FileUtils.hpp"
#include "mods/utils/RBuffer.hpp"
#include "mods/utils/RBufferBackend.hpp"
#include "mods/utils/impl/FileUtils.hpp"
#include "mods/utils/types.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
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
                       explicit FileReaderDeleter(std::vector<char>&& vec)
                         : _v(std::move(vec))
                           {
                           }
                       ~FileReaderDeleter() override = default;
                       
                       FileReaderDeleter() = delete;
                       FileReaderDeleter(const FileReaderDeleter&) = delete;
                       FileReaderDeleter(FileReaderDeleter&&) = delete;
                       auto operator=(const FileReaderDeleter&) -> FileReaderDeleter& = delete;
                       auto operator=(FileReaderDeleter&&) -> FileReaderDeleter& = delete;
                       
                     private:
                       std::vector<char> _v;
                    };
               } // namespace
             
             auto readFileToBuffer(const std::string& filename) -> RBufferBackend::ptr
               {
                  std::ifstream file(filename, std::ios::binary | std::ios::ate);
                  const std::streamsize size = file.tellg();
                  file.seekg(0, std::ios::beg);
                  
                  std::vector<char> vec(size);
                  if(file.read(vec.data(), size))
                    {
                       auto* ptr = vec.data();
                       const size_t length = vec.size();
                       auto deleter = std::make_unique<FileReaderDeleter>(std::move(vec));
                       return std::make_unique<RBufferBackend>(ptr, length, std::move(deleter));
                    }
                  return {};
               }
          } // namespace FileUtils
     } // namespace utils
} // namespace mods
