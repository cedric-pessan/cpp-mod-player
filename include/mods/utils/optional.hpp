#ifndef MODS_UTILS_OPTIONAL_HPP
#define MODS_UTILS_OPTIONAL_HPP

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
        ~optional()
          {
             cleanValue();
          }
        
        template<typename U = T>
          optional& operator=(U&& value)
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
        
        constexpr bool has_value() const noexcept
          {
             return _hasValue;
          }
        
        constexpr const T& operator*() const&
          {
             return _value.value;
          }
        
        optional(const optional&) = delete;
        optional(const optional&&) = delete;
        optional& operator=(const optional&) = delete;
        optional& operator=(const optional&&) = delete;
        
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
             Value(const Value&&) = delete;
             Value& operator=(const Value&) = delete;
             Value& operator=(const Value&&) = delete;
          } _value;
     };
   
} // namespace mods

#endif // MODS_UTILS_OPTIONAL_HPP