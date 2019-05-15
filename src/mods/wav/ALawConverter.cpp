
#include "mods/wav/ALawConverter.hpp"

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
             u8 v = static_cast<u8>(value);
             return _lookupTable[v];
          }
        
        void ALawConverter::fillLookupTable()
          {
             for(int value=0; value<std::numeric_limits<u8>::max(); ++value)
               {
                  int v = value ^ 0x55; // invert even bits
                  int exponent = (v >> MANTISSA_SIZE) & EXPONENT_MASK;
                  int mantissa = v & MANTISSA_MASK;
                  bool negative = (v & 0x80) == 0;
                  
                  s16 dest = mantissa << 1;
                  dest |= 1;
                  if(exponent > 0)
                    {
                       dest |= (1 << (MANTISSA_SIZE+1));
                       for(int i=0; i<exponent-1; ++i)
                         {
                            dest <<= 1;
                            dest |= 1;
                         }
                    }
                  dest <<= 3;
                  dest |= (dest >> 12) & 0x7;
                  if(negative) dest = -dest;
                  _lookupTable[value] = dest;
               }
          }
     } // namespace wav
} // namespace mods
