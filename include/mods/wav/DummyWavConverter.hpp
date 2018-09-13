#ifndef MODS_WAV_DUMMYCONVERTER_HPP
#define MODS_WAV_DUMMYCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class DummyWavConverter : public WavConverter
          {
           public:
             DummyWavConverter(WavConverter::ptr src);
             
             DummyWavConverter() = delete;
             DummyWavConverter(const DummyWavConverter&) = delete;
             DummyWavConverter(const DummyWavConverter&&) = delete;
             DummyWavConverter& operator=(const DummyWavConverter&) = delete;
             DummyWavConverter& operator=(const DummyWavConverter&&) = delete;
             ~DummyWavConverter() override = default;
             
             bool isFinished() const override;
             
           private:
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_DUMMYCONVERTER_HPP
