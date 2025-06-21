
#include "mods/converters/Converter.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/MuLawConverter.hpp"

#include <cstdlib>
#include <limits>
#include <string>
#include <utility>

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
             const size_t nbElems = buf->size();
             
             auto inBuf = buf->slice<u8>(0, nbElems);
             auto inView = buf->slice<s8>(0, nbElems);
             auto& outView = *buf;
             
             _src->read(&inBuf);
             
             for(size_t i=0; i<nbElems; ++i)
               {
                  const size_t idx = nbElems - 1 - i;
                  const s8 value = inView[idx];
                  outView[idx] = muLawTransform(value);
               }
          }
        
        auto MuLawConverter::muLawTransform(s8 value) const -> s16
          {
             using mods::utils::at;
             const u8 uValue = static_cast<u8>(value);
             return at(_lookupTable, uValue);
          }
        
        void MuLawConverter::fillLookupTable()
          {
             static constexpr u32 byteMask = 0xFFU;
             static constexpr u32 signMask = 0x80U;
             
             for(u32 value=0; value<std::numeric_limits<u8>::max(); ++value)
               {
                  const u32 uValue = value ^ byteMask; // invert all bits
                  const u32 exponent = (uValue >> MANTISSA_SIZE) & EXPONENT_MASK;
                  const u32 mantissa = uValue & MANTISSA_MASK;
                  const bool negative = (uValue & signMask) == 0;
                  
                  u16 dest = mantissa << 1U;
                  static constexpr u32 baseMantissa = 0b100001U;
                  dest |= baseMantissa;
                  dest <<= exponent;
                  static constexpr u32 constantOffset = 33;
                  dest -= constantOffset;
                  dest <<= 2U;
                  s16 signedDest = static_cast<s16>(dest);
                  if(negative)
                    {
                       signedDest = static_cast<s16>(-signedDest);
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
