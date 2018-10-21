#ifndef MODS_WAV_DUMMYWAVCONVERTER_HPP
#define MODS_WAV_DUMMYWAVCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class DummyWavConverter : public WavConverter
          {
           public:
             explicit DummyWavConverter(WavConverter::ptr src);
             
             DummyWavConverter() = delete;
             DummyWavConverter(const DummyWavConverter&) = delete;
             DummyWavConverter(const DummyWavConverter&&) = delete;
             DummyWavConverter& operator=(const DummyWavConverter&) = delete;
             DummyWavConverter& operator=(const DummyWavConverter&&) = delete;
             ~DummyWavConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_DUMMYWAVCONVERTER_HPP
