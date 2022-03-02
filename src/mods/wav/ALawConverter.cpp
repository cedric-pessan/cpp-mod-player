
#include "mods/wav/ALawConverter.hpp"

#include <iostream>
#include <limits>

namespace mods
{
   namespace wav
     {
        ALawConverter::ALawConverter(Converter::ptr src)
          : _src(std::move(src))
            {
               fillLookupTable();
            }
        
        auto ALawConverter::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        void ALawConverter::read(mods::utils::RWBuffer<u8>* buf, size_t len)
          {
             if((len % sizeof(s16)) != 0)
               {
                  std::cout << "TODO: wrong buffer length in ALawConverter" << std::endl;
               }
             
             size_t nbElems = len / sizeof(s16);
             size_t toReadLen = nbElems * sizeof(s8);
             
             _src->read(buf, toReadLen);
             
             auto inView = buf->slice<s8>(0, nbElems);
             auto outView = buf->slice<s16>(0, nbElems);
             
             for(size_t i=0; i<nbElems; ++i)
               {
                  size_t idx = nbElems - 1 - i;
                  s8 value = inView[idx];
                  outView[idx] = aLawTransform(value);
               }
          }
        
        auto ALawConverter::aLawTransform(s8 value) const -> s16
          {
             using mods::utils::at;
             u8 v = static_cast<u8>(value);
             return at(_lookupTable, v);
          }
        
        void ALawConverter::fillLookupTable()
          {
             static constexpr u32 evenBitsMask = 0x55U;
             static constexpr u32 signMask = 0x80U;
             
             for(u32 value=0; value<std::numeric_limits<u8>::max(); ++value)
               {
                  u32 v = value ^ evenBitsMask; // invert even bits
                  u32 exponent = (v >> MANTISSA_SIZE) & EXPONENT_MASK;
                  u32 mantissa = v & MANTISSA_MASK;
                  bool negative = (v & signMask) == 0;
                  
                  u16 dest = mantissa << 1U;
                  dest |= 1U;
                  if(exponent > 0)
                    {
                       dest |= (1U << (MANTISSA_SIZE+1));
                       for(size_t i=0; i<exponent-1; ++i)
                         {
                            dest <<= 1U;
                            dest |= 1U;
                         }
                    }
                  dest <<= 3U;
                  static constexpr u32 fillLsbBitsShift = 12U;
                  static constexpr u32 fillLsbBitsMask = 0x7U;
                  dest |= static_cast<u16>(dest >> fillLsbBitsShift) & fillLsbBitsMask;
                  
                  s16 signedDest = dest;
                  if(negative)
                    {
                       signedDest = -signedDest;
                    }
                  _lookupTable.at(value) = signedDest;
               }
          }
        
        auto ALawConverter::getBitsPerSampleRequirementsString() -> std::string&
          {
             static std::string requirements = "A-Law converter needs 8 bits per sample";
             return requirements;
          }
     } // namespace wav
} // namespace mods
