#ifndef MODS_MOD_INSTRUMENT_HPP
#define MODS_MOD_INSTRUMENT_HPP

#include "mods/utils/PackedArray.hpp"
#include "mods/utils/types.hpp"

#include <string>

namespace mods
{
   namespace mod
     {
#pragma pack(push,1)
        class Instrument
          {
           public:
             Instrument() = delete;
             Instrument(const Instrument&) = delete;
             Instrument(Instrument&&) = delete;
             auto operator=(const Instrument&) -> Instrument& = delete;
             auto operator=(Instrument&&) -> Instrument& = delete;
             ~Instrument() = delete;
             
             auto getSampleName() const -> std::string;
             
             auto getFineTune() const -> s8;
             
             auto getSampleLength() const -> u16;
             
             auto getRepeatOffset() const -> u16;
             
             auto getRepeatLength() const -> u16;
             
           private:
             static constexpr int _sampleNameLength = 22;
             
             mods::utils::PackedArray<char, _sampleNameLength> _sampleName;
             u16be _sampleLength;
             s8 _fineTune;
             u8 _volume;
             u16be _repeatOffset;
             u16be _repeatLength;
          };
#pragma pack(pop)
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_INSTRUMENT_HPP
