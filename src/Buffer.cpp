
#include "Buffer.hpp"

namespace mods
{
   Buffer::Buffer(u8* buf, size_t length, Deleter::ptr deleter)
     : _buf(buf),
     _length(length),
     _deleter(std::move(deleter))
       {
       }
          
   
   Buffer::~Buffer()
     {
     }
   
   Buffer::Deleter::Deleter()
     {
     }
   
   Buffer::Deleter::~Deleter()
     {
     }
}
