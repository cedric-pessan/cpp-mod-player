#ifndef MODS_WAV_READERWAVCONVERTER_HPP
#define MODS_WAV_READERWAVCONVERTER_HPP

#include "mods/utils/RBuffer.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/wav/StatCollector.hpp"
#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<int CH, int NBCHANNELS, int BITSPERSAMPLE>
          class ReaderWavConverter : public WavConverter
          {
           public:
             ReaderWavConverter(mods::utils::RBuffer<u8> buffer, u8 defaultValue, StatCollector::sptr statCollector);
             
             ReaderWavConverter() = delete;
             ReaderWavConverter(const ReaderWavConverter&) = delete;
             ReaderWavConverter(const ReaderWavConverter&&) = delete;
             ReaderWavConverter& operator=(const ReaderWavConverter&) = delete;
             ReaderWavConverter& operator=(const ReaderWavConverter&&) = delete;
             ~ReaderWavConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             const mods::utils::RBuffer<u8> _buffer;
             mods::utils::RBuffer<u8>::const_iterator _it;
             mods::utils::RBuffer<u8>::const_iterator _end;
             
             u8 _defaultValue;
             StatCollector::sptr _statCollector;
             
             constexpr static int BYTESPERSAMPLE = BITSPERSAMPLE/8;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_READERWAVCONVERTER_HPP
