#ifndef MODS_UTILS_ENDIANNESS_HPP
#define MODS_UTILS_ENDIANNESS_HPP

#include <cstdint>

enum struct Endianness : uint8_t
{
   BigEndian,
     LittleEndian,
     Unknown
};

#endif // MODS_UTILS_ENDIANNESS_HPP