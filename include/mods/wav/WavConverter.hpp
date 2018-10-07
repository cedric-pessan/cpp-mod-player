#ifndef MODS_WAV_WAVCONVERTER_HPP
#define MODS_WAV_WAVCONVERTER_HPP

#include <mods/utils/RBuffer.hpp>

#include <memory>

namespace mods
{
   namespace wav
     {
        class WavConverter
          {
           public:
             using ptr = std::unique_ptr<WavConverter>;
             
             static ptr buildConverter(mods::utils::RBuffer<u8> buffer, int bitsPerSample, int nbChannels, int frequency);
             
             WavConverter() = default;
             virtual ~WavConverter() = default;
             
             virtual bool isFinished() const = 0;
             
             WavConverter(const WavConverter&) = delete;
             WavConverter(const WavConverter&&) = delete;
             WavConverter& operator=(const WavConverter&) = delete;
             WavConverter& operator=(const WavConverter&&) = delete;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_WAVCONVERTER_HPP
