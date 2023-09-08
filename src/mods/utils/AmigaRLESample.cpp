
#include "mods/utils/AmigaRLESample.hpp"

namespace mods
{
   namespace utils
     {
        AmigaRLESample::AmigaRLESample(double value, u32 length, bool filtered)
          : _value(value),
          _filtered(filtered),
          _length(length)
            {
            }
        
        auto AmigaRLESample::getValue() const -> double
          {
             return _value;
          }
        
        auto AmigaRLESample::isFiltered() const -> bool
          {
             return _filtered;
          }
        
        auto AmigaRLESample::getLength() const -> u32
          {
             return _length;
          }
     } // namespace utils
} // namespace mods
