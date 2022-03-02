#ifndef MODS_WAV_ALAWCONVERTER_HPP
#define MODS_WAV_ALAWCONVERTER_HPP

#include "mods/converters/Converter.hpp"

#include <limits>

namespace mods
{
   namespace wav
     {
        class ALawConverter : public mods::converters::Converter
          {
           public:
             explicit ALawConverter(Converter::ptr src);
             
             ALawConverter() = delete;
             ALawConverter(const ALawConverter&) = delete;
             ALawConverter(ALawConverter&&) = delete;
             auto operator=(const ALawConverter&) -> ALawConverter& = delete;
             auto operator=(ALawConverter&&) -> ALawConverter& = delete;
             ~ALawConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
             static constexpr auto getZero() -> u8
               {
                  return _positiveSign | _encodedZero;
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
             static constexpr u8 _positiveSign = 0x80U;
             static constexpr u8 _encodedZero = 0x55U;
             
             static constexpr u32 MANTISSA_SIZE = 4;
             static constexpr u32 MANTISSA_MASK = 0xF;
             static constexpr u32 EXPONENT_MASK = 0x7;
             
             void fillLookupTable();
             auto aLawTransform(s8 value) const -> s16;
             
             Converter::ptr _src;
             static constexpr u32 _lookupTableSize = static_cast<u32>(std::numeric_limits<u8>::max())+1;
             std::array<s16, _lookupTableSize> _lookupTable {};
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_ALAWCONVERTER_HPP
