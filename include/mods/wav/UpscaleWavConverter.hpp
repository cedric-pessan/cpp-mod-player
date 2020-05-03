#ifndef MODS_WAV_UPSCALEWAVCONVERTER_HPP
#define MODS_WAV_UPSCALEWAVCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename TOut, typename TIn>
          class UpscaleWavConverter : public WavConverter
          {
           public:
             explicit UpscaleWavConverter(WavConverter::ptr src);
             
             UpscaleWavConverter() = delete;
             UpscaleWavConverter(const UpscaleWavConverter&) = delete;
             UpscaleWavConverter(UpscaleWavConverter&&) = delete;
             auto operator=(const UpscaleWavConverter&) -> UpscaleWavConverter& = delete;
             auto operator=(UpscaleWavConverter&&) -> UpscaleWavConverter& = delete;
             ~UpscaleWavConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             constexpr auto shiftLeftValue() -> u32
               {
                  return sizeof(TOut)*BITS_IN_BYTE - sizeof(TIn)*BITS_IN_BYTE;
               }
             
             constexpr auto shiftRightValue() -> u32
               {
                  return sizeof(TIn)*BITS_IN_BYTE - shiftLeftValue();
               }
             
             constexpr auto maskValue() -> u32
               {
                  return (1U << shiftLeftValue()) - 1;
               }
             
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_UPSCALEWAVCONVERTER_HPP
