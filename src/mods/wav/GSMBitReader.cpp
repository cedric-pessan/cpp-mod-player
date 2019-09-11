
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
        
        int GSMBitReader::read(int bits)
          {
             int value = 0;
             
             for(int i=0; i<bits; ++i)
               {
                  int bytePos = _position / 8;
                  u8 byte = _buf[bytePos];
                  
                  int bitInByte = _position & 7;
                  u8 bitMask = 1 << bitInByte;
                  
                  if(byte & bitMask)
                    {
                       value |= (1 << i);
                    }
                  
                  ++_position;
               }
             return value;
          }
     } // namespace wav
} // namespace mods
