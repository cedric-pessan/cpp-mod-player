#ifndef MODS_WAV_ALAWCONVERTER_HPP
#define MODS_WAV_ALAWCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class ALawConverter : public WavConverter
          {
           public:
             explicit ALawConverter(WavConverter::ptr src);
             
             ALawConverter() = delete;
             ALawConverter(const ALawConverter&) = delete;
             ALawConverter(ALawConverter&&) = delete;
             ALawConverter& operator=(const ALawConverter&) = delete;
             ALawConverter& operator=(ALawConverter&&) = delete;
             ~ALawConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
             static constexpr u8 getZero()
               {
                  return 0x80 | 0x55;
               }
             
           private:
             static constexpr int MANTISSA_SIZE = 4;
             static constexpr int MANTISSA_MASK = 0xF;
             static constexpr int EXPONENT_MASK = 0x7;
             
             void fillLookupTable();
             s16 aLawTransform(s8 value) const;
             
             WavConverter::ptr _src;
             std::array<s16, 256> _lookupTable;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_ALAWCONVERTER_HPP
