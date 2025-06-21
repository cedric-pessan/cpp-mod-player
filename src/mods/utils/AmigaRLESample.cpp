
#include "mods/utils/AmigaRLESample.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace utils
     {
        AmigaRLESample::AmigaRLESample(double value, SampleLength length, bool filtered)
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
