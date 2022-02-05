#ifndef MODS_UTILS_TYPES_HPP
#define MODS_UTILS_TYPES_HPP

#include "endianness.hpp"
#include "mods/utils/PackedArray.hpp"
#include "mods/utils/nativeEndian.hpp"

#include <cassert>
#include <climits>
#include <cstdint>
#include <type_traits>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

constexpr u32 BITS_IN_BYTE = CHAR_BIT;

#pragma pack(push,1)
template<Endianness endianness>
  class u16leImpl
{
 private:
   mods::utils::PackedArray<u8, 2> _v;
   
 public:
   explicit operator u16() const
     {
        return static_cast<u16>(_v[0]) |
          static_cast<u16>(static_cast<u16>(_v[1]) << BITS_IN_BYTE);
     }
   
   u16leImpl() = delete;
   u16leImpl(const u16leImpl&) = delete;
   u16leImpl(u16leImpl&&) = delete;
   auto operator=(const u16leImpl&) -> u16leImpl& = delete;
   auto operator=(u16leImpl&&) -> u16leImpl& = delete;
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
   u16leImpl(u16leImpl&&) = delete;
   auto operator=(const u16leImpl&) -> u16leImpl& = delete;
   auto operator=(u16leImpl&&) -> u16leImpl& = delete;
   ~u16leImpl() = delete;
};

template<Endianness endianness>
  class u16beImpl
{
 private:
   mods::utils::PackedArray<u8, 2> _v;
   
 public:
   explicit operator u16() const
     {
        return static_cast<u16>(static_cast<u16>(_v[0]) << BITS_IN_BYTE) |
          static_cast<u16>(_v[1]);
     }
   
   u16beImpl() = delete;
   u16beImpl(const u16beImpl&) = delete;
   u16beImpl(u16beImpl&&) = delete;
   auto operator=(const u16beImpl&) -> u16beImpl& = delete;
   auto operator=(u16beImpl&&) -> u16beImpl& = delete;
   ~u16beImpl() = delete;
};

template<>
  class u16beImpl<Endianness::BigEndian>
{
 private:
   u16 _value;
   
 public:
   explicit operator u16() const
     {
        return _value;
     }
   
   u16beImpl() = delete;
   u16beImpl(const u16beImpl&) = delete;
   u16beImpl(u16beImpl&&) = delete;
   auto operator=(const u16beImpl&) -> u16beImpl& = delete;
   auto operator=(u16beImpl&&) -> u16beImpl& = delete;
   ~u16beImpl() = delete;
};

template<Endianness endianness>
  class u32leImpl
{
 private:
   mods::utils::PackedArray<u8, 4> _v;
   
 public:
   explicit operator u32() const
     {
        u32 value = 0;
        for(int i=0; i<4; ++i)
          {
             value |= (static_cast<u32>(_v[i]) << (BITS_IN_BYTE * i));
          }
        return value;
     }
   
   u32leImpl() = delete;
   u32leImpl(const u32leImpl&) = delete;
   u32leImpl(u32leImpl&&) = delete;
   auto operator=(const u32leImpl&) -> u32leImpl& = delete;
   auto operator=(u32leImpl&&) -> u32leImpl& = delete;
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
   u32leImpl(u32leImpl&&) = delete;
   auto operator=(const u32leImpl&) -> u32leImpl& = delete;
   auto operator=(u32leImpl&&) -> u32leImpl& = delete;
   ~u32leImpl() = delete;
};

using u16le = u16leImpl<NativeEndianness::value>;
using u32le = u32leImpl<NativeEndianness::value>;
using u16be = u16beImpl<NativeEndianness::value>;

class s16le 
{
 private:
   u16le _value;
   
 public:
   explicit operator s16() const
     {
        return static_cast<s16>(static_cast<u16>(_value));
     }
   
   s16le() = delete;
   s16le(const s16le&) = delete;
   s16le(s16le&&) = delete;
   auto operator=(const s16le&) -> s16le& = delete;
   auto operator=(s16le&&) -> s16le& = delete;
   ~s16le() = delete;
};

class s16be
{
 private:
   u16be _value;
   
 public:
   s16be() = delete;
   s16be(const s16be&) = delete;
   s16be(s16be&&) = delete;
   auto operator=(const s16be&) -> s16be& = delete;
   auto operator=(s16be&&) -> s16be& = delete;
   ~s16be() = delete;
};

#pragma pack(pop)

template<typename E>
  constexpr auto toUnderlying(E e) noexcept
{
   return static_cast<std::underlying_type_t<E>>(e);
}

namespace mods
{
   namespace utils
     {
        template<typename T>
          auto at(const T& a, std::size_t i) -> const typename T::value_type&
            {
               assert(i >= 0 && i < a.size());
               return a[i]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
            }
        
        template<typename T>
          auto at(T& a, std::size_t i) -> typename T::value_type&
            {
               assert(i >= 0 && i < a.size());
               return a[i]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
            }
        
        template<typename T>
          constexpr auto clamp(T v, T lo, T hi) -> T
            {
               if(v < lo)
                 {
                    return lo;
                 }
               if(v > hi)
                 {
                    return hi;
                 }
               return v;
            }
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_TYPES_HPP
