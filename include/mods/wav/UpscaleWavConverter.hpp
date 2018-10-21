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
             UpscaleWavConverter(const UpscaleWavConverter&&) = delete;
             UpscaleWavConverter& operator=(const UpscaleWavConverter&) = delete;
             UpscaleWavConverter& operator=(const UpscaleWavConverter&&) = delete;
             ~UpscaleWavConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             constexpr u32 shiftLeftValue()
               {
                  return sizeof(TOut)*8 - sizeof(TIn)*8;
               }
             
             constexpr u32 shiftRightValue()
               {
                  return sizeof(TIn)*8 - shiftLeftValue();
               }
             
             constexpr u32 maskValue()
               {
                  return (1U << shiftLeftValue()) - 1;
               }
             
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_UPSCALEWAVCONVERTER_HPP
