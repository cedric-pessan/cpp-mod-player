
#include "mods/converters/DivideConverter.hpp"

#include <iostream>

namespace mods
{
   namespace converters
     {
        DivideConverter::DivideConverter(Converter::ptr src, double coef)
          : _src(std::move(src)),
          _coef(coef)
            {
            }
        
        auto DivideConverter::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        void DivideConverter::read(mods::utils::RWBuffer<u8>* buf, size_t len)
          {
             if((len % sizeof(double)) != 0)
               {
                  std::cout << "TODO: wrong buffer length in DivideConverter" << std::endl;
               }
             
             size_t nbElems = len / sizeof(double);
             
             _src->read(buf, len);
             
             auto view = buf->slice<double>(0, nbElems);
             
             for(size_t i=0; i<nbElems; ++i)
               {
                  view[i] /= _coef;
               }
          }
     } // namespace converters
} // namespace mods