
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
             return _lookupTable.at(v);
          }
        
        void MuLawConverter::fillLookupTable()
          {
             for(u32 value=0; value<std::numeric_limits<u8>::max(); ++value)
               {
                  u32 v = value ^ 0xFFu; // invert all bits
                  u32 exponent = (v >> MANTISSA_SIZE) & EXPONENT_MASK;
                  u32 mantissa = v & MANTISSA_MASK;
                  bool negative = (v & 0x80u) == 0;
                  
                  u16 dest = mantissa << 1u;
                  dest |= 0b100001u;
                  dest <<= exponent;
                  dest -= 33;
                  dest <<= 2u;
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
