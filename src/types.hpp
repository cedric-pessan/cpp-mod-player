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
  class u16leImpl
{
 private:
   u8 _v[2];
   
 private:
   u16leImpl() = delete;
   u16leImpl(const u16leImpl&) = delete;
   u16leImpl& operator=(const u16leImpl&) = delete;
   ~u16leImpl();
};

template<>
  class u16leImpl<Endianness::LittleEndian>
{
 private:
   u16 _value;
   
 private:
   u16leImpl() = delete;
   u16leImpl(const u16leImpl&) = delete;
   u16leImpl& operator=(const u16leImpl&) = delete;
   ~u16leImpl();
};

template<Endianness endianness>
  class u32leImpl
{
 private:
   u8 _v[4];
   
 public:
   operator u32() const
     {
        return static_cast<u32>(_v[0]) |
          (static_cast<u32>(_v[1]) << 8) |
          (static_cast<u32>(_v[2]) << 16) |
          (static_cast<u32>(_v[3]) << 24);
     }
   
 private:
   u32leImpl() = delete;
   u32leImpl(const u32leImpl&) = delete;
   u32leImpl& operator=(const u32leImpl&) = delete;
   ~u32leImpl();
};

template<>
  class u32leImpl<Endianness::LittleEndian>
{
 private:
   u32 _value;
   
 public:
   operator u32() const
     {
        return _value;
     }
   
 private:
   u32leImpl() = delete;
   u32leImpl(const u32leImpl&) = delete;
   u32leImpl& operator=(const u32leImpl&) = delete;
   ~u32leImpl();
};

typedef u16leImpl<NativeEndianness::value> u16le;
typedef u32leImpl<NativeEndianness::value> u32le;

#endif // _TYPES_HPP_
