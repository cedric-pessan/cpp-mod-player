#ifndef MODS_WAV_GSMINT16_HPP
#define MODS_WAV_GSMINT16_HPP

#include "mods/utils/arithmeticShifter.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace wav
     {
        class GSMInt16
          {
           public:
             GSMInt16() = default;
             GSMInt16(const GSMInt16&) = default;
             GSMInt16(GSMInt16&&) = default;
             GSMInt16& operator=(const GSMInt16&) = default;
             GSMInt16& operator=(GSMInt16&&) = default;
             ~GSMInt16() = default;
             
             constexpr GSMInt16(int value) noexcept // NOLINT(hicpp-explicit-conversions,google-explicit-constructor)
               : _value(value)
                 {
                    signExtend();
                 }
             
             GSMInt16& operator=(int value) noexcept
               {
                  _value = value;
                  signExtend();
                  return *this;
               }
             
             s32 getValue() const
               {
                  return _value;
               }
             
             bool operator==(const GSMInt16& v) const
               {
                  return _value == v._value;
               }
             bool operator>(const GSMInt16& v) const
               {
                  return _value > v._value;
               }
             bool operator<(const GSMInt16& v) const
               {
                  return _value < v._value;
               }
             
             GSMInt16 operator>>(const GSMInt16& shift) const
               {
                  if(shift < 0)
                    {
                       return *this << -shift;
                    }
                  auto n = static_cast<u32>(shift.getValue());
                  return mods::utils::arithmeticShifter::shiftRight(_value, n);
               }
             GSMInt16 operator<<(const GSMInt16& shift) const
               {
                  if(shift < 0)
                    {
                       return *this >> -shift;
                    }
                  u32 x = static_cast<u32>(_value);
                  auto n = static_cast<u32>(shift.getValue());
                  return x << n;
               }
             
             GSMInt16 operator-() const
               {
                  return 0 - _value;
               }
             
             GSMInt16 operator+(const GSMInt16& v) const
               {
                  auto res = _value + v._value;
                  if(res > 32767)
                    {
                       res = 32767;
                    }
                  else if(res < -32768)
                    {
                       res = -32768;
                    }
                  return res;
               }
             GSMInt16 operator-(const GSMInt16& v) const
               {
                  auto res = _value - v._value;
                  if(res > 32767)
                    {
                       res = 32767;
                    }
                  else if(res < -32768)
                    {
                       res = -32768;
                    }
                  return res;
               }
             
             GSMInt16 mult_round(const GSMInt16& v) const
               {
                  if(_value == -32768 && v._value == -32768)
                    {
                       return 32767;
                    }
                  s32 res = _value * v._value + 16384;
                  
                  return mods::utils::arithmeticShifter::shiftRight(res, 15);
               }
             
             GSMInt16 abs() const
               {
                  if(_value == -32768)
                    {
                       return 32767;
                    }
                  if(_value < 0)
                    {
                       return -_value;
                    }
                  return *this;
               }
             
             GSMInt16& operator--()
               {
                  auto res = *this - 1;
                  _value = res._value;
                  return *this;
               }
             
             GSMInt16& operator<<=(const GSMInt16& v)
               {
                  auto res = *this << v;
                  _value = res._value;
                  return *this;
               }
             
             GSMInt16& operator>>=(const GSMInt16& v)
               {
                  auto res = *this >> v;
                  _value = res._value;
                  return *this;
               }
             
             GSMInt16& operator+=(const GSMInt16& v)
               {
                  auto res = *this + v;
                  _value = res._value;
                  return *this;
               }
             
             GSMInt16& operator-=(const GSMInt16& v)
               {
                  auto res = *this - v;
                  _value = res._value;
                  return *this;
               }
             
           private:
             constexpr void signExtend()
               {
                  _value = static_cast<s32>(static_cast<s16>(_value));
               }
             
             s32 _value = 0;
          };
        
        inline GSMInt16 operator<<(int value, const GSMInt16& shift)
          {
             return GSMInt16(value) << shift;
          }
        
        inline GSMInt16 operator-(int left, const GSMInt16& right)
          {
             return GSMInt16(left) - right;
          }
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_GSMINT16_HPP
