
#include "mods/wav/MuLawConverter.hpp"

#include <limits>

namespace mods
{
   namespace wav
     {
        MuLawConverter::MuLawConverter(WavConverter::ptr src)
          : _src(std::move(src))
            {
               fillLookupTable();
            }
        
        bool MuLawConverter::isFinished() const
          {
             return _src->isFinished();
          }
        
        void MuLawConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             if((len % sizeof(s16)) != 0)
               {
                  std::cout << "TODO: wrong buffer length in MuLawConverter" << std::endl;
               }
             
             int nbElems = len / sizeof(s16);
             int toReadLen = nbElems * sizeof(s8);
             
             _src->read(buf, toReadLen);
             
             auto inView = buf->slice<s8>(0, nbElems);
             auto outView = buf->slice<s16>(0, nbElems);
             
             for(int i = nbElems-1; i>=0; --i)
               {
                  s8 value = inView[i];
                  outView[i] = muLawTransform(value);
               }
          }
        
        s16 MuLawConverter::muLawTransform(s8 value) const
          {
             u8 v = static_cast<u8>(value);
             return _lookupTable[v];
          }
        
        void MuLawConverter::fillLookupTable()
          {
             for(int value=0; value<std::numeric_limits<u8>::max(); ++value)
               {
                  int v = value ^ 0xFF; // invert all bits
                  int exponent = (v >> MANTISSA_SIZE) & EXPONENT_MASK;
                  int mantissa = v & MANTISSA_MASK;
                  bool negative = (v & 0x80) == 0;
                  
                  s16 dest = mantissa << 1;
                  dest |= 0b100001;
                  dest <<= exponent;
                  dest -= 33;
                  dest <<= 2;
                  if(negative) dest = -dest;
                  _lookupTable[value] = dest;
               }
          }
     } // namespace wav
} // namespace mods
