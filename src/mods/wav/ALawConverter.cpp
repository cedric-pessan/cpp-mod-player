
#include "mods/wav/ALawConverter.hpp"

#include <cmath>
#include <limits>

namespace mods
{
   namespace wav
     {
        template<typename T>
          ALawConverter<T>::ALawConverter(WavConverter::ptr src)
            : _src(std::move(src))
              {
                 fillLookupTable();
              }
        
        template<typename T>
          bool ALawConverter<T>::isFinished() const
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void ALawConverter<T>::read(mods::utils::RWBuffer<u8>* buf, int len)
            {
               if((len % sizeof(T)) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in ALawConverter" << std::endl;
                 }
               
               int nbElems = len / sizeof(T);
               int toReadLen = nbElems * sizeof(s8);
               
               _src->read(buf, toReadLen);
               
               auto inView = buf->slice<s8>(0, nbElems);
               auto outView = buf->slice<T>(0, nbElems);
               
               for(int i = nbElems-1; i>=0; --i)
                 {
                    s8 value = inView[i];
                    aLawTransform(&outView[i], value);
                 }
            }
        
        template<typename T>
          void ALawConverter<T>::aLawTransform(T* out, s8 value) const
          {
             u8 v = static_cast<u8>(value);
             *out = _lookupTable[v];
          }
        
        template<typename T>
          void ALawConverter<T>::fillLookupTable()
            {
               double A = 87.6;
               double bound = 1.0 / (1.0 + std::log(A));
               
               for(int v=-128; v<=127; ++v)
                 {
                    double sign;
                    double vabs;
                    if(v >= 0)
                      {
                         sign = 1.0;
                         vabs = v / static_cast<double>(std::numeric_limits<s8>::max());
                      }
                    else
                      {
                         sign = -1.0;
                         vabs = v / static_cast<double>(std::numeric_limits<s8>::min());
                      }
                    if(vabs < bound)
                      {
                         vabs = vabs * (1.0 + std::log(A)) / A;
                      }
                    else
                      {
                         vabs = std::exp(vabs * (1.0 + std::log(A)) - 1.0) / A;
                      }
                    int idx = static_cast<u8>(v);
                    _lookupTable[idx] = sign * vabs;
                 }
            }
        
        template class ALawConverter<double>;
     } // namespace wav
} // namespace mods
