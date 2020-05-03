#ifndef MODS_WAV_UNSIGNEDTOSIGNEDWAVCONVERTER_HPP
#define MODS_WAV_UNSIGNEDTOSIGNEDWAVCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename T>
          class UnsignedToSignedWavConverter : public WavConverter
          {
           public:
             explicit UnsignedToSignedWavConverter(WavConverter::ptr src);
             
             UnsignedToSignedWavConverter() = delete;
             UnsignedToSignedWavConverter(const UnsignedToSignedWavConverter&) = delete;
             UnsignedToSignedWavConverter(UnsignedToSignedWavConverter&&) = delete;
             auto operator=(const UnsignedToSignedWavConverter&) -> UnsignedToSignedWavConverter& = delete;
             auto operator=(UnsignedToSignedWavConverter&&) -> UnsignedToSignedWavConverter& = delete;
             ~UnsignedToSignedWavConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             constexpr auto getOffset() -> int
               {
                  return (1U << (sizeof(T) * BITS_IN_BYTE)) / 2;
               }
             
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_UNSIGNEDTOSIGNEDWAVCONVERTER_HPP
