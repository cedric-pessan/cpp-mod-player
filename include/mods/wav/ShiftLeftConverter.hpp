#ifndef MODS_WAV_SHIFTLEFTCONVERTER_HPP
#define MODS_WAV_SHIFTLEFTCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class ShiftLeftConverter : public WavConverter
          {
           public:
             ShiftLeftConverter(WavConverter::ptr);
             
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
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_SHIFTLEFTCONVERTER_HPP
