#ifndef _RBUFFER_HPP_
#define _RBUFFER_HPP_

#include <iostream>

namespace mods
{
   template<typename T>
   class RBuffer
     {
      public:
        RBuffer()
          {
          }
        ~RBuffer()
          {
          }
        
        T* operator->() const
          {
             std::cout << "TODO: RBuffer<T>::operator->() const" << std::endl;
             return nullptr;
          }
        
      private:
        RBuffer(const RBuffer&);
        RBuffer& operator=(const RBuffer&);
     };
}

#endif // _RBUFFER_HPP_