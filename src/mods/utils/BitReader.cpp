
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
	  auto BitReader<BYTESWAP, BITORDER>::read(size_t bits) -> u32
	    {
	       u32 value = 0;
	       
	       for(size_t i=0; i<bits; ++i)
		 {
		    size_t bytePos = _position / BITS_IN_BYTE;
		    if(BYTESWAP == ByteSwap::U32) 
		      {
			 bytePos ^= 3U;
		      }
		    u8 byte = _buf[bytePos];
		    
                    static constexpr u32 bitPositionMask = 0x7U;
		    size_t bitInByte = _position & bitPositionMask;
		    u8 bitMask = 0;
		    if(BITORDER == BitOrder::LsbToMsb)
		      {
			 bitMask = 1U << bitInByte;
		      }
		    else
		      {
			 bitMask = 1U << ((BITS_IN_BYTE - 1) - bitInByte);
			 value <<= 1U;
		      }
		    
		    if((byte & bitMask) != 0)
		      {
			 if(BITORDER == BitOrder::LsbToMsb)
			   {
			      value |= (1U << i);
			   }
			 else
			   {
			      value |= 1U;
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
