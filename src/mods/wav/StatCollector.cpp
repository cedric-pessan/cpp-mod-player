
#include "mods/wav/StatCollector.hpp"

#include <cstddef>

namespace mods
{
   namespace wav
     {
        void StatCollector::inc(size_t newBytesRead)
          {
             _value += newBytesRead;
          }
        
        auto StatCollector::getBytesRead() const -> size_t
          {
             return _value;
          }
     } //namespace wav
} // namespace mods
