#ifndef MODS_UTILS_BITREADER_HPP
#define MODS_UTILS_BITREADER_HPP

namespace mods
{
   namespace utils
     {
        class BitReader
          {
           public:
             BitReader() = default;
             BitReader(const BitReader&) = delete;
             BitReader(BitReader&&) = delete;
             BitReader& operator=(const BitReader&) = delete;
             BitReader& operator=(BitReader&&) = delete;
             ~BitReader() = default;
             
             void reset();
             
           private:
             int _position = 0;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_BITREADER_HPP
