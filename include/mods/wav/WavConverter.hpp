#ifndef MODS_WAV_WAVCONVERTER_HPP
#define MODS_WAV_WAVCONVERTER_HPP

#include <memory>

namespace mods
{
   namespace wav
     {
        class WavConverter
          {
           public:
             using ptr = std::unique_ptr<WavConverter>;
             
             static ptr buildConverter(int bitsPerSample);
             
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
