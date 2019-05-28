#ifndef MODS_WAV_MULAWCONVERTER_HPP
#define MODS_WAV_MULAWCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class MuLawConverter : public WavConverter
          {
           public:
             explicit MuLawConverter(WavConverter::ptr src);
             
             MuLawConverter() = delete;
             MuLawConverter(const MuLawConverter&) = delete;
             MuLawConverter(MuLawConverter&&) = delete;
             MuLawConverter& operator=(const MuLawConverter&) = delete;
             MuLawConverter& operator=(MuLawConverter&&) = delete;
             ~MuLawConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
             static constexpr u8 getZero()
               {
                  return 0xff;
               }
             
           private:
             static constexpr int MANTISSA_SIZE = 4;
             static constexpr int MANTISSA_MASK = 0xF;
             static constexpr int EXPONENT_MASK = 0x7;
             
             void fillLookupTable();
             s16 muLawTransform(s8 value) const;
             
             WavConverter::ptr _src;
             std::array<s16, 256> _lookupTable;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_MULAWCONVERTER_HPP
