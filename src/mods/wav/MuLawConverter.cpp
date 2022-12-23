
#include "mods/wav/MuLawConverter.hpp"

#include <iostream>
#include <limits>

namespace mods
{
   namespace wav
     {
        MuLawConverter::MuLawConverter(Converter<u8>::ptr src)
          : _src(std::move(src))
            {
               fillLookupTable();
            }
        
        auto MuLawConverter::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        void MuLawConverter::read(mods::utils::RWBuffer<s16>* buf)
          {
             size_t nbElems = buf->size();
             
             auto inBuf = buf->slice<u8>(0, nbElems);
             auto inView = buf->slice<s8>(0, nbElems);
             auto& outView = *buf;
             
             _src->read(&inBuf);
             
             for(size_t i=0; i<nbElems; ++i)
               {
                  size_t idx = nbElems - 1 - i;
                  s8 value = inView[idx];
                  outView[idx] = muLawTransform(value);
               }
          }
        
        auto MuLawConverter::muLawTransform(s8 value) const -> s16
          {
             using mods::utils::at;
             u8 v = static_cast<u8>(value);
             return at(_lookupTable, v);
          }
        
        void MuLawConverter::fillLookupTable()
          {
             static constexpr u32 byteMask = 0xFFU;
             static constexpr u32 signMask = 0x80U;
             
             for(u32 value=0; value<std::numeric_limits<u8>::max(); ++value)
               {
                  u32 v = value ^ byteMask; // invert all bits
                  u32 exponent = (v >> MANTISSA_SIZE) & EXPONENT_MASK;
                  u32 mantissa = v & MANTISSA_MASK;
                  bool negative = (v & signMask) == 0;
                  
                  u16 dest = mantissa << 1U;
                  static constexpr u32 baseMantissa = 0b100001U;
                  dest |= baseMantissa;
                  dest <<= exponent;
                  static constexpr u32 constantOffset = 33;
                  dest -= constantOffset;
                  dest <<= 2U;
                  s16 signedDest = dest;
                  if(negative)
                    {
                       signedDest = -signedDest;
                    }
                  _lookupTable.at(value) = signedDest;
               }
          }
        
        auto MuLawConverter::getBitsPerSampleRequirementsString() -> std::string&
          {
             static std::string requirements = "Mu-Law converter needs 8 bits per sample";
             return requirements;
          }
     } // namespace wav
} // namespace mods
