
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
     } // namespace mod
} // namespace mods
