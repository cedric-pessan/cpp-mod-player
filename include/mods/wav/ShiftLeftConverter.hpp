#ifndef MODS_WAV_SHIFTLEFTCONVERTER_HPP
#define MODS_WAV_SHIFTLEFTCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename T>
          class ShiftLeftConverter : public WavConverter
          {
           public:
             ShiftLeftConverter(WavConverter::ptr src, u32 shiftBy);
             
             ShiftLeftConverter() = delete;
             ShiftLeftConverter(const ShiftLeftConverter&) = delete;
             ShiftLeftConverter(ShiftLeftConverter&&) = delete;
             ShiftLeftConverter& operator=(const ShiftLeftConverter&) = delete;
             ShiftLeftConverter& operator=(ShiftLeftConverter&&) = delete;
             ~ShiftLeftConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             WavConverter::ptr _src;
             u32 _shiftLeft;
             u32 _shiftRight;
             u32 _maskRight;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_SHIFTLEFTCONVERTER_HPP
