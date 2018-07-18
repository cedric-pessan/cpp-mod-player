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
          {
          }
        ~optional()
          {
          }
        
        template<typename U = T>
          optional& operator=(U&& value)
            {
               return *this;
            }
        
      private:
        optional(const optional&) = delete;
        optional& operator=(const optional&) = delete;
     };
   
}

#endif // _OPTIONAL_HPP_
