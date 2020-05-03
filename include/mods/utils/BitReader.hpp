#ifndef MODS_UTILS_BITREADER_HPP
#define MODS_UTILS_BITREADER_HPP

#include "mods/utils/RBuffer.hpp"

#include <cstddef>

namespace mods
{
   namespace utils
     {
	enum struct ByteSwap
	  {
	     None,
	       U32
	  };
	
	enum struct BitOrder
	  {
	     LsbToMsb,
	       MsbToLsb
	  };
	
	template<ByteSwap BYTESWAP, BitOrder BITORDER>
	  class BitReader
          {
           public:
             explicit BitReader(const mods::utils::RBuffer<u8>& buf);
             
             BitReader() = delete;
             BitReader(const BitReader&) = delete;
             BitReader(BitReader&&) = delete;
             auto operator=(const BitReader&) -> BitReader& = delete;
             auto operator=(BitReader&&) -> BitReader& = delete;
             ~BitReader() = default;
             
             void reset();
             
             auto read(size_t bits) -> u32;
             
           private:
             size_t _position = 0;
             const mods::utils::RBuffer<u8>& _buf;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_BITREADER_HPP
