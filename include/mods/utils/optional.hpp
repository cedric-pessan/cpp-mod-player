#ifndef MODS_UTILS_OPTIONAL_HPP
#define MODS_UTILS_OPTIONAL_HPP

#include "mods/utils/types.hpp"

#include <utility>

namespace mods
{
   /*
    * very basic optional class to be removed once we can switch to c++17
    */
   
   template<typename T>
     class optional
     {
      public:
        constexpr optional() noexcept
          : _value()
          {
          }
        
        template<typename... Args>
          constexpr explicit optional(Args&&... args)
            : _hasValue(true)
              {
                 ::new(&_value.value)T(std::forward<Args>(args)...); // NOLINT(cppcoreguidelines-pro-type-union-access)
              }
        
        ~optional()
          {
             cleanValue();
          }
        
        template<typename U = T>
          auto operator=(U&& value) -> optional&
            {
               cleanValue();
               if(_hasValue) 
                 {
                    _value.value = std::forward<T>(value); // NOLINT(cppcoreguidelines-pro-type-union-access)
                 }
               else 
                 {
                    ::new(&_value.value)T(std::forward<T>(value)); // NOLINT(cppcoreguidelines-pro-type-union-access)
                 }
               _hasValue = true;
               return *this;
            }
        
        constexpr auto has_value() const noexcept -> bool
          {
             return _hasValue;
          }
        
        constexpr auto operator*() const& -> const T&
          {
             return _value.value; // NOLINT(cppcoreguidelines-pro-type-union-access)
          }
        
        constexpr auto operator->() const -> const T*
          {
             return std::addressof(_value.value); // NOLINT(cppcoreguidelines-pro-type-union-access)
          }
        
        optional(optional&& other) noexcept
          : _value()
            {
               if(other._hasValue)
                 {
                    *this = std::move(other._value.value); // NOLINT(cppcoreguidelines-pro-type-union-access)
                    other._hasValue = false;
                 }
            }
        
        auto operator=(optional&& other) noexcept -> optional&
          {
             cleanValue();
             _hasValue = false;
             if(other._hasValue)
               {
                  *this = std::move(other._value.value); // NOLINT(cppcoreguidelines-pro-type-union-access)
                  other._hasValue = false;
               }
             return *this;
          }
        
        optional(const optional&) = delete;
        auto operator=(const optional&) -> optional& = delete;
        
      private:
        void cleanValue()
          {
             if(_hasValue)
               {
                  _value.value.~T(); // NOLINT(cppcoreguidelines-pro-type-union-access)
               }
          }
        
        bool _hasValue = false;
        union Value 
          {
             T value;
             u8 empty;
             
             constexpr Value()
               : empty(0)
               {
               }
             ~Value()
               {
                  empty = 0;
               }
             
             Value(const Value&) = delete;
             Value(Value&&) = delete;
             auto operator=(const Value&) -> Value& = delete;
             auto operator=(Value&&) -> Value& = delete;
          } _value;
     };
   
} // namespace mods

#endif // MODS_UTILS_OPTIONAL_HPP
