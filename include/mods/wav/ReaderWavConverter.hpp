#ifndef MODS_WAV_READERWAVCONVERTER_HPP
#define MODS_WAV_READERWAVCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class ReaderWavConverter : public WavConverter
          {
           public:
             ReaderWavConverter() = default;
             ReaderWavConverter(const ReaderWavConverter&) = delete;
             ReaderWavConverter(const ReaderWavConverter&&) = delete;
             ReaderWavConverter& operator=(const ReaderWavConverter&) = delete;
             ReaderWavConverter& operator=(const ReaderWavConverter&&) = delete;
             ~ReaderWavConverter() override = default;
             
             bool isFinished() const override;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_READERWAVCONVERTER_HPP
