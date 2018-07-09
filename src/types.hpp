#ifndef _TYPES_HPP_
#define _TYPES_HPP_

#include "endianness.hpp"
#include "nativeEndian.hpp"

#include <stdint.h>
#include <iostream>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

template<Endianness endianness>
  class u32leImpl
{
 private:
   u8 v[4];
   
 public:
   operator u32() const
     {
        return static_cast<u32>(v[0]) |
          (static_cast<u32>(v[1]) << 8) |
          (static_cast<u32>(v[2]) << 16) |
          (static_cast<u32>(v[3]) << 24);
     }
   
 private:
   u32leImpl() = delete;
   u32leImpl(const u32leImpl&) = delete;
   u32leImpl& operator=(const u32leImpl&) = delete;
   ~u32leImpl();
};

/*template<>
  class u32leImpl<NativeEndianness::value>;*/ //TODO

typedef u32leImpl<NativeEndianness::value> u32le;

#endif // _TYPES_HPP_
