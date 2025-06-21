
#include "mods/mod/Instrument.hpp"
#include "mods/utils/types.hpp"

#include <cstddef>
#include <string>

namespace mods
{
   namespace mod
     {
        auto Instrument::getSampleName() const -> std::string
          {
             size_t length = 0;
             for(auto character : _sampleName)
               {
                  if(character == '\0')
                    {
                       break;
                    }
                  ++length;
               }
             return { _sampleName.data(), length };
          }
        
        auto Instrument::getSampleLength() const -> u16
          {
             return static_cast<u16>(_sampleLength) * 2;
          }
        
        auto Instrument::getFineTune() const -> s8
          {
             constexpr u8 signBitMask = 0x8U;
             constexpr u8 valueMask = 7U;
             
             if((_fineTune & signBitMask) == 0)
               {
                  return static_cast<s8>(_fineTune & valueMask);
               }
             const u8 absoluteValueFineTune = _fineTune & valueMask;
             return static_cast<s8>(-((absoluteValueFineTune ^ valueMask) + 1));
          }
        
        auto Instrument::getVolume() const -> u8
          {
             return _volume;
          }
        
        auto Instrument::getRepeatOffset() const -> u16
          {
             return static_cast<u16>(_repeatOffset) * 2;
          }
        
        auto Instrument::getRepeatLength() const -> u16
          {
             auto off = getRepeatOffset();
             if(off >= getSampleLength())
               {
                  return 0;
               }
             
             auto length = static_cast<u16>(_repeatLength) * 2;
             if((off + length) >= getSampleLength()) 
               {
                  length = getSampleLength() - off;
               }
             return length;
          }
     } // namespace mod
} // namespace mods
