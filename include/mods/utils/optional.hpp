#ifndef _OPTIONAL_HPP_
#define _OPTIONAL_HPP_

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
          : _hasValue(false)
          {
          }
        ~optional()
          {
          }
        
        template<typename U = T>
          optional& operator=(U&& value)
            {
               _hasValue = true;
               return *this;
            }
        
        constexpr bool has_value() const noexcept
          {
             return _hasValue;
          }
        
      private:
        optional(const optional&) = delete;
        optional& operator=(const optional&) = delete;
        
        bool _hasValue;
     };
   
}

#endif // _OPTIONAL_HPP_
