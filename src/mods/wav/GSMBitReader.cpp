
#include "mods/utils/types.hpp"
#include "mods/wav/GSMBitReader.hpp"

namespace mods
{
   namespace wav
     {
        GSMBitReader::GSMBitReader(const mods::utils::RBuffer<u8>& buf)
          : _buf(buf)
            {
            }
        
        void GSMBitReader::reset()
          {
             _position = 0;
          }
        
        int GSMBitReader::read(size_t bits)
          {
             u32 value = 0;
             
             for(size_t i=0; i<bits; ++i)
               {
                  int bytePos = _position / 8;
                  u8 byte = _buf[bytePos];
                  
                  size_t bitInByte = _position & 7u;
                  u8 bitMask = 1u << bitInByte;
                  
                  if((byte & bitMask) != 0)
                    {
                       value |= (1u << i);
                    }
                  
                  ++_position;
               }
             return value;
          }
     } // namespace wav
} // namespace mods
