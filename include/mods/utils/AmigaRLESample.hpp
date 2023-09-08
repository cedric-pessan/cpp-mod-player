#ifndef MODS_UTILS_AMIGARLESAMPLE_HPP
#define MODS_UTILS_AMIGARLESAMPLE_HPP

#include "mods/utils/types.hpp"

namespace mods
{
   namespace utils
     {
        class AmigaRLESample
          {
           public:
             AmigaRLESample(double value, u32 length, bool filtered);
             
             AmigaRLESample() = default;
             AmigaRLESample(const AmigaRLESample&) = default;
             AmigaRLESample(AmigaRLESample&&) = default;
             auto operator=(const AmigaRLESample&) -> AmigaRLESample& = default;
             auto operator=(AmigaRLESample&&) -> AmigaRLESample& = default;
             ~AmigaRLESample() = default;
             
             auto isFiltered() const -> bool;
             auto getValue() const -> double;
             auto getLength() const -> u32;
             
           private:
             double _value;
             bool _filtered;
             u32 _length;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_AMIGARLESAMPLE_HPP
