#ifndef MODS_UTILS_BITREADER_HPP
#define MODS_UTILS_BITREADER_HPP

#include "mods/utils/RBuffer.hpp"

namespace mods
{
   namespace utils
     {
        class BitReader
          {
           public:
             explicit BitReader(const mods::utils::RBuffer<u8>& buf);
             
             BitReader() = delete;
             BitReader(const BitReader&) = delete;
             BitReader(BitReader&&) = delete;
             BitReader& operator=(const BitReader&) = delete;
             BitReader& operator=(BitReader&&) = delete;
             ~BitReader() = default;
             
             void reset();
             
             int read(size_t bits);
             
           private:
             size_t _position = 0;
             const mods::utils::RBuffer<u8>& _buf;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_BITREADER_HPP
