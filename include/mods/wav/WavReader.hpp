#ifndef MODS_WAV_WAVREADER_HPP
#define MODS_WAV_WAVREADER_HPP

#include "mods/converters/Converter.hpp"
#include "mods/ModuleReader.hpp"
#include "mods/utils/RBuffer.hpp"
#include "mods/wav/Format.hpp"
#include "mods/wav/StatCollector.hpp"
#include "mods/wav/WavTypes.hpp"

namespace mods
{
   namespace wav
     {
        class WavReader : public ModuleReader
          {
           public:
             explicit WavReader(const std::string& filename);
             ~WavReader() override = default;
             
             WavReader() = delete;
             WavReader(const WavReader&) = delete;
             WavReader(WavReader&&) = delete;
             auto operator=(const WavReader&) -> WavReader& = delete;
             auto operator=(WavReader&&) -> WavReader& = delete;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<s16>* buf) override;
             auto getInfo() const -> std::string override;
             auto getProgressInfo() const -> std::string override;
             
           private:
             static auto readFMT(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                 const mods::utils::RBuffer<u8>& riffBuffer,
                                 size_t offset) -> Format;
             static auto readFact(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                  const mods::utils::RBuffer<u8>& riffBuffer,
                                  size_t offset) -> mods::utils::RBuffer<FactHeader>;
             static auto readData(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                  const mods::utils::RBuffer<u8>& riffBuffer,
                                  size_t offset) -> mods::utils::RBuffer<u8>;
             
             static void readDisp(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                  const mods::utils::RBuffer<u8>& riffBuffer,
                                  size_t offset,
                                  std::stringstream& description);
             
             static void parseInfoList(const mods::utils::RBuffer<ListHeader>& listHeader,
                                       const mods::utils::RBuffer<u8>& riffBuffer,
                                       size_t offset,
                                       std::stringstream& description);
             
             static void readAfsp(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                  const mods::utils::RBuffer<u8>& riffBuffer,
                                  size_t offset,
                                  std::stringstream& description);
             void readCue() const;
             void parseAdtl() const;
             static auto readPeak(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                  const mods::utils::RBuffer<u8>& riffBuffer,
                                  size_t offset,
                                  int nbChannels) -> double;
             
             void buildInfo(int bitsPerSample, int bitsPerContainer, int nbChannels, int frequency, const std::string& description, WavAudioFormat codec);
             
             mods::converters::Converter<s16>::ptr _converter;
             const mods::utils::RBuffer<u8> _fileBuffer;
             std::string _info;
             StatCollector _statCollector;
             size_t _length;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_WAVREADER_HPP