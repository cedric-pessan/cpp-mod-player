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
             auto operator=(const DownscaleWavConverter&) -> DownscaleWavConverter& = delete;
             auto operator=(DownscaleWavConverter&&) -> DownscaleWavConverter& = delete;
             ~DownscaleWavConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             auto allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>;
             void ensureTempBufferSize(size_t len);
             
             auto constexpr shiftRight() -> u32
               {
                  return (sizeof(TIn) - sizeof(TOut))*BITS_IN_BYTE;
               }
             
             WavConverter::ptr _src;
             
             std::vector<u8> _tempVec;
             mods::utils::RWBuffer<u8> _temp;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_DOWNSCALEWAVCONVERTER_HPP
