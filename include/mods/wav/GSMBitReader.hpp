#ifndef MODS_WAV_GSMBITREADER_HPP
#define MODS_WAV_GSMBITREADER_HPP

#include "mods/utils/RBuffer.hpp"

namespace mods
{
   namespace wav
     {
        class GSMBitReader
          {
           public:
             explicit GSMBitReader(const mods::utils::RBuffer<u8>& buf);
             
             GSMBitReader() = delete;
             GSMBitReader(const GSMBitReader&) = delete;
             GSMBitReader(GSMBitReader&&) = delete;
             GSMBitReader& operator=(const GSMBitReader&) = delete;
             GSMBitReader& operator=(GSMBitReader&&) = delete;
             ~GSMBitReader() = default;
             
             void reset();
             
             int read(size_t bits);
             
           private:
             size_t _position = 0;
             const mods::utils::RBuffer<u8>& _buf;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_GSMBITREADER_HPP