
#include "mods/mod/Instrument.hpp"

namespace mods
{
   namespace mod
     {
        auto Instrument::getSampleName() const -> std::string
          {
             size_t length = 0;
             for(auto c : _sampleName)
               {
                  if(c == '\0')
                    {
                       break;
                    }
                  ++length;
               }
             return std::string(_sampleName.data(), length);
          }
        
        auto Instrument::getSampleLength() const -> size_t
          {
             return static_cast<u16>(_sampleLength) * 2;
          }
     } // namespace mod
} // namespace mods
