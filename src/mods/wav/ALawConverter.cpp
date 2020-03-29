
#include "mods/wav/ALawConverter.hpp"

#include <iostream>
#include <limits>

namespace mods
{
   namespace wav
     {
        ALawConverter::ALawConverter(WavConverter::ptr src)
          : _src(std::move(src))
            {
               fillLookupTable();
            }
        
        bool ALawConverter::isFinished() const
          {
             return _src->isFinished();
          }
        
        void ALawConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             if((len % sizeof(s16)) != 0)
               {
                  std::cout << "TODO: wrong buffer length in ALawConverter" << std::endl;
               }
             
             int nbElems = len / sizeof(s16);
             int toReadLen = nbElems * sizeof(s8);
             
             _src->read(buf, toReadLen);
             
             auto inView = buf->slice<s8>(0, nbElems);
             auto outView = buf->slice<s16>(0, nbElems);
             
             for(int i = nbElems-1; i>=0; --i)
               {
                  s8 value = inView[i];
                  outView[i] = aLawTransform(value);
               }
          }
        
        s16 ALawConverter::aLawTransform(s8 value) const
          {
             using mods::utils::at;
             u8 v = static_cast<u8>(value);
             return at(_lookupTable, v);
          }
        
        void ALawConverter::fillLookupTable()
          {
             for(u32 value=0; value<std::numeric_limits<u8>::max(); ++value)
               {
                  u32 v = value ^ 0x55u; // invert even bits
                  int exponent = (v >> MANTISSA_SIZE) & EXPONENT_MASK;
                  u32 mantissa = v & MANTISSA_MASK;
                  bool negative = (v & 0x80u) == 0;
                  
                  u16 dest = mantissa << 1u;
                  dest |= 1u;
                  if(exponent > 0)
                    {
                       dest |= (1u << (MANTISSA_SIZE+1));
                       for(int i=0; i<exponent-1; ++i)
                         {
                            dest <<= 1u;
                            dest |= 1u;
                         }
                    }
                  dest <<= 3u;
                  dest |= static_cast<u16>(dest >> 12u) & 0x7u;
                  
                  s16 signedDest = dest;
                  if(negative)
                    {
                       signedDest = -signedDest;
                    }
                  _lookupTable.at(value) = signedDest;
               }
          }
     } // namespace wav
} // namespace mods
