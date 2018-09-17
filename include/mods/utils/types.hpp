#ifndef MODS_UTILS_TYPES_HPP
#define MODS_UTILS_TYPES_HPP

#include "endianness.hpp"
#include "mods/utils/nativeEndian.hpp"

#include <cstdint>
#include <iostream>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

#pragma pack(push,1)
template<Endianness endianness>
  class u16leImpl
{
 private:
   u8 _v[2];
   
 public:
   explicit operator u16() const
     {
        return static_cast<u16>(_v[0]) |
          static_cast<u16>(static_cast<u16>(_v[1]) << 8u);
     }
   
   u16leImpl() = delete;
   u16leImpl(const u16leImpl&) = delete;
   u16leImpl(const u16leImpl&&) = delete;
   u16leImpl& operator=(const u16leImpl&) = delete;
   u16leImpl& operator=(const u16leImpl&&) = delete;
   ~u16leImpl() = delete;
};

template<>
  class u16leImpl<Endianness::LittleEndian>
{
 private:
   u16 _value;
   
 public:
   explicit operator u16() const
     {
        return _value;
     }
   
   u16leImpl() = delete;
   u16leImpl(const u16leImpl&) = delete;
   u16leImpl(const u16leImpl&&) = delete;
   u16leImpl& operator=(const u16leImpl&) = delete;
   u16leImpl& operator=(const u16leImpl&&) = delete;
   ~u16leImpl() = delete;
};

template<Endianness endianness>
  class u32leImpl
{
 private:
   u8 _v[4];
   
 public:
   explicit operator u32() const
     {
        return static_cast<u32>(_v[0]) |
          (static_cast<u32>(_v[1]) << 8u) |
          (static_cast<u32>(_v[2]) << 16u) |
          (static_cast<u32>(_v[3]) << 24u);
     }
   
   u32leImpl() = delete;
   u32leImpl(const u32leImpl&) = delete;
   u32leImpl(const u32leImpl&&) = delete;
   u32leImpl& operator=(const u32leImpl&) = delete;
   u32leImpl& operator=(const u32leImpl&&) = delete;
   ~u32leImpl() = delete;
};

template<>
  class u32leImpl<Endianness::LittleEndian>
{
 private:
   u32 _value;
   
 public:
   explicit operator u32() const
     {
        return _value;
     }
   
   u32leImpl() = delete;
   u32leImpl(const u32leImpl&) = delete;
   u32leImpl(const u32leImpl&&) = delete;
   u32leImpl& operator=(const u32leImpl&) = delete;
   u32leImpl& operator=(const u32leImpl&&) = delete;
   ~u32leImpl() = delete;
};
#pragma pack(pop)

using u16le = u16leImpl<NativeEndianness::value>;
using u32le = u32leImpl<NativeEndianness::value>;

template<typename E>
  constexpr auto toUnderlying(E e) noexcept
{
   return static_cast<std::underlying_type_t<E>>(e);
}

#endif // MODS_UTILS_TYPES_HPP
