
#include "mods/wav/DivideConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        DivideConverter::DivideConverter(WavConverter::ptr src, double coef)
          : _src(std::move(src)),
          _coef(coef)
            {
            }
        
        bool DivideConverter::isFinished() const
          {
             return _src->isFinished();
          }
        
        void DivideConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             if((len % sizeof(double)) != 0)
               {
                  std::cout << "TODO: wrong buffer length in DivideConverter" << std::endl;
               }
             
             int nbElems = len / sizeof(double);
             
             _src->read(buf, len);
             
             auto view = buf->slice<double>(0, nbElems);
             
             for(int i=0; i<nbElems; ++i)
               {
                  view[i] /= _coef;
               }
          }
     } // namespace wav
} // namespace mods