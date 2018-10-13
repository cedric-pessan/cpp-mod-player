#ifndef MODS_WAV_UNSIGNEDTOSIGNEDWAVCONVERTER_HPP
#define MODS_WAV_UNSIGNEDTOSIGNEDWAVCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class UnsignedToSignedWavConverter : public WavConverter
          {
           public:
             UnsignedToSignedWavConverter(WavConverter::ptr src);
             
             UnsignedToSignedWavConverter() = delete;
             UnsignedToSignedWavConverter(const UnsignedToSignedWavConverter&) = delete;
             UnsignedToSignedWavConverter(const UnsignedToSignedWavConverter&&) = delete;
             UnsignedToSignedWavConverter& operator=(const UnsignedToSignedWavConverter&) = delete;
             UnsignedToSignedWavConverter& operator=(const UnsignedToSignedWavConverter&&) = delete;
             ~UnsignedToSignedWavConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>& buf, int len) override;
             
           private:
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_UNSIGNEDTOSIGNEDWAVCONVERTER_HPP
