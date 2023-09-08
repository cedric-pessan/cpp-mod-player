
#include "mods/mod/Note.hpp"

#include <iostream>

namespace mods
{
   namespace mod
     {
        auto Note::getInstrument() const -> size_t
          {
             return (_value[0] & 0xF0) | (_value[2]>> 4);
          }
        
        auto Note::getPeriod() const -> u16
          {
             return (static_cast<u16>(_value[0]) << 8) | static_cast<u16>(_value[1]);
          }
     } // namespace mod
} // namespace mods
