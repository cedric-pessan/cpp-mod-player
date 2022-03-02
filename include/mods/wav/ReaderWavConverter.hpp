#ifndef MODS_WAV_READERWAVCONVERTER_HPP
#define MODS_WAV_READERWAVCONVERTER_HPP

#include "mods/converters/Converter.hpp"
#include "mods/wav/StatCollector.hpp"

namespace mods
{
   namespace wav
     {
        class ReaderWavConverter : public mods::converters::Converter
          {
           public:
             ReaderWavConverter(mods::utils::RBuffer<u8> buffer, u8 defaultValue, StatCollector* statCollector);
             
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
             StatCollector* _statCollector;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_READERWAVCONVERTER_HPP
