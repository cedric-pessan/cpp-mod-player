
#include "mods/utils/BitReader.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace utils
     {
        BitReader::BitReader(const mods::utils::RBuffer<u8>& buf)
          : _buf(buf)
            {
            }
        
        void BitReader::reset()
          {
             _position = 0;
          }
        
        int BitReader::read(int bits)
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
     } // namespace utils
} // namespace mods
