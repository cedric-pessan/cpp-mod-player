#ifndef MODS_WAV_MULAWCONVERTER_HPP
#define MODS_WAV_MULAWCONVERTER_HPP

#include "mods/converters/Converter.hpp"

#include <limits>

namespace mods
{
   namespace wav
     {
        class MuLawConverter : public mods::converters::Converter<s16>
          {
           public:
             explicit MuLawConverter(Converter<u8>::ptr src);
             
             MuLawConverter() = delete;
             MuLawConverter(const MuLawConverter&) = delete;
             MuLawConverter(MuLawConverter&&) = delete;
             auto operator=(const MuLawConverter&) -> MuLawConverter& = delete;
             auto operator=(MuLawConverter&&) -> MuLawConverter& = delete;
             ~MuLawConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<s16>* buf) override;
             
             static constexpr auto getZero() -> u8
               {
                  return _zero;
               }
             
             static constexpr auto isValidAsBitsPerSample(int bitsPerSample) -> bool
               {
                  return bitsPerSample == BITS_IN_BYTE;
               }
             
             static constexpr auto getOutputBitsPerSample() -> int
               {
                  constexpr int outputBitsPerSample = 16;
                  return outputBitsPerSample;
               }
             
             static auto getBitsPerSampleRequirementsString() -> std::string&;
             
           private:
             static constexpr u32 MANTISSA_SIZE = 4;
             static constexpr u32 MANTISSA_MASK = 0xF;
             static constexpr u32 EXPONENT_MASK = 0x7;
             
             static constexpr u8 _zero = 0xff;
             
             void fillLookupTable();
             auto muLawTransform(s8 value) const -> s16;
             
             Converter<u8>::ptr _src;
             static constexpr u32 _lookupTableSize = static_cast<u32>(std::numeric_limits<u8>::max())+1;
             std::array<s16, _lookupTableSize> _lookupTable {};
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_MULAWCONVERTER_HPP
