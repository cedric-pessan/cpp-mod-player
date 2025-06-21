
#include "mods/converters/Converter.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/ALawConverter.hpp"

#include <cstddef>
#include <limits>
#include <string>
#include <utility>

namespace mods
{
   namespace wav
     {
        ALawConverter::ALawConverter(Converter<u8>::ptr src)
          : _src(std::move(src))
            {
               fillLookupTable();
            }
        
        auto ALawConverter::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        void ALawConverter::read(mods::utils::RWBuffer<s16>* buf)
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
                  outView[idx] = aLawTransform(value);
               }
          }
        
        auto ALawConverter::aLawTransform(s8 value) const -> s16
          {
             using mods::utils::at;
             const u8 uValue = static_cast<u8>(value);
             return at(_lookupTable, uValue);
          }
        
        void ALawConverter::fillLookupTable()
          {
             static constexpr u32 evenBitsMask = 0x55U;
             static constexpr u32 signMask = 0x80U;
             
             for(u32 value=0; value<std::numeric_limits<u8>::max(); ++value)
               {
                  const u32 invValue = value ^ evenBitsMask; // invert even bits
                  const u32 exponent = (invValue >> MANTISSA_SIZE) & EXPONENT_MASK;
                  const u32 mantissa = invValue & MANTISSA_MASK;
                  const bool negative = (invValue & signMask) == 0;
                  
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
                  
                  s16 signedDest = static_cast<s16>(dest);
                  if(negative)
                    {
                       signedDest = static_cast<s16>(-signedDest);
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
