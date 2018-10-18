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
             UpscaleWavConverter(WavConverter::ptr src);
             
             UpscaleWavConverter() = delete;
             UpscaleWavConverter(const UpscaleWavConverter&) = delete;
             UpscaleWavConverter(const UpscaleWavConverter&&) = delete;
             UpscaleWavConverter& operator=(const UpscaleWavConverter&) = delete;
             UpscaleWavConverter& operator=(const UpscaleWavConverter&&) = delete;
             ~UpscaleWavConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>& buf, int len) override;
             
           private:
             constexpr int shiftLeftValue()
               {
                  return sizeof(TOut)*8 - sizeof(TIn)*8;
               }
             
             constexpr int shiftRightValue()
               {
                  return sizeof(TIn)*8 - shiftLeftValue();
               }
             
             constexpr int maskValue()
               {
                  return (1 << shiftLeftValue()) - 1;
               }
             
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_UPSCALEWAVCONVERTER_HPP
