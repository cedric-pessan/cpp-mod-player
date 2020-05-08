#ifndef MODS_WAV_WAVCONVERTER_HPP
#define MODS_WAV_WAVCONVERTER_HPP

#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/StatCollector.hpp"
#include "mods/wav/WavTypes.hpp"

#include <memory>
#include <vector>

namespace mods
{
   namespace wav
     {
        class WavConverter
          {
           public:
             using ptr = std::unique_ptr<WavConverter>;
             
             static auto buildConverter(const mods::utils::RBuffer<u8>& buffer, int bitsPerSample, int bitsPerContainer, int nbChannels, int frequency, const StatCollector::sptr& statCollector, WavAudioFormat codec, u32 channelMask, double peak) -> ptr;
             
             WavConverter() = default;
             virtual ~WavConverter() = default;
             
             virtual auto isFinished() const -> bool = 0;
             virtual void read(mods::utils::RWBuffer<u8>* buf, size_t len) = 0;
             
             WavConverter(const WavConverter&) = delete;
             WavConverter(WavConverter&&) = delete;
             auto operator=(const WavConverter&) -> WavConverter& = delete;
             auto operator=(WavConverter&&) -> WavConverter& = delete;
             
           private:
             static auto isResamplableByPositiveIntegerFactor(int frequency) -> bool;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_WAVCONVERTER_HPP
