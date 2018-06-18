
#include "../../Buffer.hpp"

namespace mods
{
   namespace FileUtils
     {
        namespace
          {
             class UnixMapperDeleter : public Buffer::Deleter
               {
                public:
                  UnixMapperDeleter()
                    {
                    }
                  virtual ~UnixMapperDeleter()
                    {
                    }
                  
                private:
                  UnixMapperDeleter(const UnixMapperDeleter&);
                  UnixMapperDeleter& operator=(const UnixMapperDeleter&);
               };
          }
        
        Buffer::sptr mapFileToBuffer()
          {
             std::cout << "TODO: FileUtils::mapFileToBuffer()" << std::endl;
             auto deleter = std::make_unique<UnixMapperDeleter>();
             return std::make_shared<Buffer>(nullptr, 0, std::move(deleter));
          }
     }
}
