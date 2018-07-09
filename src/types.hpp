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
 public:
   operator u32() const
     {
        std::cout << "TODO: u32leImpl::operatoru32() const" << std::endl;
        return 0;
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
