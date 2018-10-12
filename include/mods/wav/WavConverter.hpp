#ifndef MODS_WAV_WAVCONVERTER_HPP
#define MODS_WAV_WAVCONVERTER_HPP

#include <mods/utils/RBuffer.hpp>
#include <mods/utils/RWBuffer.hpp>

#include <memory>

namespace mods
{
   namespace wav
     {
        class WavConverter
          {
           public:
             using ptr = std::unique_ptr<WavConverter>;
             
             static ptr buildConverter(const mods::utils::RBuffer<u8>& buffer, int bitsPerSample, int nbChannels, int frequency);
             
             WavConverter() = default;
             virtual ~WavConverter() = default;
             
             virtual bool isFinished() const = 0;
             virtual void read(mods::utils::RWBuffer<u8>& buf, int len) = 0;
             
             WavConverter(const WavConverter&) = delete;
             WavConverter(const WavConverter&&) = delete;
             WavConverter& operator=(const WavConverter&) = delete;
             WavConverter& operator=(const WavConverter&&) = delete;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_WAVCONVERTER_HPP
