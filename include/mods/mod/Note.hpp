#ifndef MODS_MOD_NOTE_HPP
#define MODS_MOD_NOTE_HPP

#include "mods/utils/PackedArray.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace mod
     {
#pragma pack(push,1)
        class Note
          {
           public:
             Note() = delete;
             Note(const Note&) = delete;
             Note(Note&&) = delete;
             auto operator=(const Note&) = delete;
             auto operator=(Note&&) = delete;
             ~Note() = delete;
             
             auto getInstrument() const -> size_t;
             
           private:
             constexpr static size_t noteSize = 4;
             
             mods::utils::PackedArray<u8, noteSize> _value;
          };
#pragma pack(pop)
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_NOTE_HPP
