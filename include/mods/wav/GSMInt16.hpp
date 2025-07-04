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
             auto operator=(const GSMInt16&) -> GSMInt16& = default;
             auto operator=(GSMInt16&&) -> GSMInt16& = default;
             ~GSMInt16() = default;
             
             constexpr GSMInt16(int value) noexcept // NOLINT(hicpp-explicit-conversions,google-explicit-constructor)
               : _value(value)
                 {
                    signExtend();
                 }
             
             auto operator=(int value) noexcept -> GSMInt16&
               {
                  _value = value;
                  signExtend();
                  return *this;
               }
             
             auto getValue() const -> s32
               {
                  return _value;
               }
             
             auto operator==(const GSMInt16& right) const -> bool
               {
                  return _value == right._value;
               }
             auto operator>(const GSMInt16& right) const -> bool
               {
                  return _value > right._value;
               }
             auto operator<(const GSMInt16& right) const -> bool
               {
                  return _value < right._value;
               }
             
             auto operator>>(const GSMInt16& shift) const -> GSMInt16
               {
                  if(shift < 0)
                    {
                       return doLeftShift(-shift);
                    }
                  return doRightShift(shift);
               }
             auto operator<<(const GSMInt16& shift) const -> GSMInt16
               {
                  if(shift < 0)
                    {
                       return doRightShift(-shift);
                    }
                  return doLeftShift(shift);
               }
             
             auto operator-() const -> GSMInt16
               {
                  return 0 - _value;
               }
             
             auto operator+(const GSMInt16& right) const -> GSMInt16
               {
                  auto res = _value + right._value;
                  if(res > _max)
                    {
                       res = _max;
                    }
                  else if(res < _min)
                    {
                       res = _min;
                    }
                  return res;
               }
             auto operator-(const GSMInt16& right) const -> GSMInt16
               {
                  auto res = _value - right._value;
                  if(res > _max)
                    {
                       res = _max;
                    }
                  else if(res < _min)
                    {
                       res = _min;
                    }
                  return res;
               }
             
             auto mult_round(const GSMInt16& right) const -> GSMInt16
               {
                  using mods::utils::arithmeticShifter::Shift;
                  
                  if(_value == _min && right._value == _min)
                    {
                       return _max;
                    }
                  const s32 res = (_value * right._value) + _roundConstant;
                  
                  static constexpr int fixedPointShiftAfterMult = 15;
                  return mods::utils::arithmeticShifter::shiftRight(res, static_cast<Shift>(fixedPointShiftAfterMult));
               }
             
             auto abs() const -> GSMInt16
               {
                  if(_value == _min)
                    {
                       return _max;
                    }
                  if(_value < 0)
                    {
                       return -_value;
                    }
                  return *this;
               }
             
             auto operator--() -> GSMInt16&
               {
                  auto res = *this - 1;
                  _value = res._value;
                  return *this;
               }
             
             auto operator<<=(const GSMInt16& right) -> GSMInt16&
               {
                  auto res = *this << right;
                  _value = res._value;
                  return *this;
               }
             
             auto operator>>=(const GSMInt16& right) -> GSMInt16&
               {
                  auto res = *this >> right;
                  _value = res._value;
                  return *this;
               }
             
             auto operator+=(const GSMInt16& right) -> GSMInt16&
               {
                  auto res = *this + right;
                  _value = res._value;
                  return *this;
               }
             
             auto operator-=(const GSMInt16& right) -> GSMInt16&
               {
                  auto res = *this - right;
                  _value = res._value;
                  return *this;
               }
             
           private:
             constexpr void signExtend()
               {
                  _value = static_cast<s32>(static_cast<s16>(_value));
               }
             
             auto doLeftShift(const GSMInt16& shift) const -> GSMInt16
               {
                  const u32 xTmp = static_cast<u32>(_value);
                  auto nShift = static_cast<u32>(shift.getValue());
                  return static_cast<int>(xTmp << nShift);
               }
             
             auto doRightShift(const GSMInt16& shift) const -> GSMInt16
               {
                  using mods::utils::arithmeticShifter::Shift;
                  
                  auto nShift = static_cast<u32>(shift.getValue());
                  return mods::utils::arithmeticShifter::shiftRight(_value, static_cast<Shift>(nShift));
               }
             
             s32 _value = 0;
             
             static constexpr s32 _max = 32767;
             static constexpr s32 _min = -32768;
             static constexpr s32 _roundConstant = 16384;
          };
        
        inline auto operator<<(int value, const GSMInt16& shift) -> GSMInt16
          {
             return GSMInt16(value) << shift;
          }
        
        inline auto operator-(int left, const GSMInt16& right) -> GSMInt16
          {
             return GSMInt16(left) - right;
          }
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_GSMINT16_HPP
