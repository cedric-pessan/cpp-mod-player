
#include "mods/mod/Instrument.hpp"

namespace mods
{
   namespace mod
     {
        auto Instrument::getSampleName() const -> std::string
          {
             size_t length = 0;
             for(auto c : _sampleName)
               {
                  if(c == '\0')
                    {
                       break;
                    }
                  ++length;
               }
             return std::string(_sampleName.data(), length);
          }
        
        auto Instrument::getSampleLength() const -> u16
          {
             return static_cast<u16>(_sampleLength) * 2;
          }
        
        auto Instrument::getFineTune() const -> s8
          {
             s8 v = _fineTune & 0xF;
             if(v & 0x8)
               {
                  v |= 0xF0;
               }
             
             return v;
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
