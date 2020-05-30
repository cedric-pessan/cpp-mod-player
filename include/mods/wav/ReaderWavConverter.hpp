#ifndef MODS_WAV_READERWAVCONVERTER_HPP
#define MODS_WAV_READERWAVCONVERTER_HPP

#include "mods/wav/StatCollector.hpp"
#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class ReaderWavConverter : public WavConverter
          {
           public:
             ReaderWavConverter(mods::utils::RBuffer<u8> buffer, u8 defaultValue, StatCollector::sptr statCollector);
             
             ReaderWavConverter() = delete;
             ReaderWavConverter(const ReaderWavConverter&) = delete;
             ReaderWavConverter(ReaderWavConverter&&) = delete;
             auto operator=(const ReaderWavConverter&) -> ReaderWavConverter& = delete;
             auto operator=(ReaderWavConverter&&) -> ReaderWavConverter& = delete;
             ~ReaderWavConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             const mods::utils::RBuffer<u8> _buffer;
             mods::utils::RBuffer<u8>::const_iterator _it;
             mods::utils::RBuffer<u8>::const_iterator _end;
             
             u8 _defaultValue;
             StatCollector::sptr _statCollector;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_READERWAVCONVERTER_HPP
