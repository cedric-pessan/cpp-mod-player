#ifndef MODS_WAV_WAVREADER_HPP
#define MODS_WAV_WAVREADER_HPP

#include "WavConverter.hpp"
#include "mods/ModuleReader.hpp"
#include "mods/utils/RBuffer.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace wav
     {
        class Format;
        
        class WavReader : public ModuleReader
          {
           public:
             explicit WavReader(const std::string& filename);
             ~WavReader() override = default;
             
             WavReader() = delete;
             WavReader(const WavReader&) = delete;
             WavReader(const WavReader&&) = delete;
             WavReader& operator=(const WavReader&) = delete;
             WavReader& operator=(const WavReader&&) = delete;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             std::string getInfo() const override;
             std::string getProgressInfo() const override;
             
           private:
             Format readFMT(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                            const mods::utils::RBuffer<u8>& riffBuffer,
                            size_t offset) const;
             mods::utils::RBuffer<FactHeader> readFact(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                                       const mods::utils::RBuffer<u8>& riffBuffer,
                                                       size_t offset) const;
             mods::utils::RBuffer<u8> readData(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                               const mods::utils::RBuffer<u8>& riffBuffer,
                                               size_t offset) const;
             
             void readDisp(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                           const mods::utils::RBuffer<u8>& riffBuffer,
                           size_t offset,
                           std::stringstream& description) const;
             
             void parseInfoList(const mods::utils::RBuffer<ListHeader>& listHeader,
                                const mods::utils::RBuffer<u8>& riffBuffer,
                                size_t offset,
                                std::stringstream& description) const;
             
             void readAfsp(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                           const mods::utils::RBuffer<u8>& riffBuffer,
                           size_t offset,
                           std::stringstream& description) const;
             void readCue() const;
             void parseAdtl() const;
             double readPeak(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                             const mods::utils::RBuffer<u8>& riffBuffer,
                             size_t offset,
                             int nbChannels) const;
             
             void buildInfo(int bitsPerSample, int nbChannels, int frequency, const std::string& description, WavAudioFormat codec);
             
             
             WavConverter::ptr _converter;
             const mods::utils::RBuffer<u8> _fileBuffer;
             std::string _info;
             StatCollector::sptr _statCollector;
             size_t _length;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_WAVREADER_HPP