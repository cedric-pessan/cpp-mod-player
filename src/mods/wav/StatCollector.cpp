
#include "mods/wav/StatCollector.hpp"

namespace mods
{
   namespace wav
     {
        void StatCollector::inc(size_t i)
          {
             _value += i;
          }
        
        auto StatCollector::getBytesRead() const -> size_t
          {
             return _value;
          }
     } //namespace wav
} // namespace mods
