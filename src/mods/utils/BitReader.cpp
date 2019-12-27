
#include "mods/utils/BitReader.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace utils
     {
	template<ByteSwap BYTESWAP, BitOrder BITORDER>
	  BitReader<BYTESWAP, BITORDER>::BitReader(const mods::utils::RBuffer<u8>& buf)
	    : _buf(buf)
	      {
	      }
        
	template<ByteSwap BYTESWAP, BitOrder BITORDER>
	  void BitReader<BYTESWAP, BITORDER>::reset()
	    {
	       _position = 0;
	    }
        
	template<ByteSwap BYTESWAP, BitOrder BITORDER>
	  int BitReader<BYTESWAP, BITORDER>::read(size_t bits)
	    {
	       u32 value = 0;
	       
	       for(size_t i=0; i<bits; ++i)
		 {
		    int bytePos = _position / 8;
		    if(BYTESWAP == ByteSwap::U32) 
		      {
			 bytePos ^= 3;
		      }
		    u8 byte = _buf[bytePos];
		    
		    size_t bitInByte = _position & 7u;
		    u8 bitMask = 0;
		    if(BITORDER == BitOrder::LsbToMsb)
		      {
			 bitMask = 1u << bitInByte;
		      }
		    else
		      {
			 bitMask = 1u << (7 - bitInByte);
			 value <<= 1;
		      }
		    
		    if((byte & bitMask) != 0)
		      {
			 if(BITORDER == BitOrder::LsbToMsb)
			   {
			      value |= (1u << i);
			   }
			 else
			   {
			      value |= 1;
			   }
		      }
		    
		    ++_position;
		 }
	       return value;
	    }
	
	template class BitReader<ByteSwap::U32, BitOrder::MsbToLsb>;
	template class BitReader<ByteSwap::None, BitOrder::LsbToMsb>;
     } // namespace utils
} // namespace mods
