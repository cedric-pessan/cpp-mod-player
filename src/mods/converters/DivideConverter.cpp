
#include "mods/converters/DivideConverter.hpp"

#include <iostream>

namespace mods
{
   namespace converters
     {
        DivideConverter::DivideConverter(Converter<double>::ptr src, double coef)
          : _src(std::move(src)),
          _coef(coef)
            {
            }
        
        auto DivideConverter::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        void DivideConverter::read(mods::utils::RWBuffer<double>* buf)
          {
             _src->read(buf);
             
             for(auto& elem : *buf)
               {
                  elem /= _coef;
               }
          }
     } // namespace converters
} // namespace mods