#ifndef MODS_WAV_DOWNSCALEWAVCONVERTER_HPP
#define MODS_WAV_DOWNSCALEWAVCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename TOut, typename TIn>
          class DownscaleWavConverter : public WavConverter
          {
           public:
             explicit DownscaleWavConverter(WavConverter::ptr src);
             
             DownscaleWavConverter() = delete;
             DownscaleWavConverter(const DownscaleWavConverter&) = delete;
             DownscaleWavConverter(DownscaleWavConverter&&) = delete;
             DownscaleWavConverter& operator=(const DownscaleWavConverter&) = delete;
             DownscaleWavConverter& operator=(DownscaleWavConverter&&) = delete;
             ~DownscaleWavConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             mods::utils::RWBuffer<u8> allocateNewTempBuffer(size_t len);
             void ensureTempBufferSize(size_t len);
             
             constexpr u32 shiftRight()
               {
                  return (sizeof(TIn) - sizeof(TOut))*8;
               }
             
             WavConverter::ptr _src;
             
             std::vector<u8> _tempVec;
             mods::utils::RWBuffer<u8> _temp;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_DOWNSCALEWAVCONVERTER_HPP
