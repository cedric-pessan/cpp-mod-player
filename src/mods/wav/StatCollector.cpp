
#include "mods/wav/StatCollector.hpp"

namespace mods
{
   namespace wav
     {
        StatCollector::StatCollector()
          : _value(0)
            {
            }
        
        void StatCollector::inc(size_t i)
          {
             _value += i;
          }
        
        size_t StatCollector::getBytesRead() const
          {
             return _value;
          }
     } //namespace wav
} // namespace mods
