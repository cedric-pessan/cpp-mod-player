#ifndef MODS_WAV_UPSCALEWAVCONVERTER_HPP
#define MODS_WAV_UPSCALEWAVCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
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
             
           private:
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_UPSCALEWAVCONVERTER_HPP
