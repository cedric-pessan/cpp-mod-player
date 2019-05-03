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
        enum struct WavAudioFormat : u16
          {
             PCM = 0x0001
          };
        
        enum struct DispType : u32
          {
             TEXT = 1
          };
        
#pragma pack(push, 1)
        struct ChunkHeader
          {
           private:
             char chunkID[4];
             u32le chunkSize;
             
           public:
             std::string getChunkID() const noexcept
               {
                  return std::string(static_cast<const char*>(chunkID), sizeof(chunkID));
               }
             
             u32 getChunkSize() const noexcept
               {
                  return static_cast<u32>(chunkSize);
               }
          };
        
        struct RiffHeader
          {
           public:
             ChunkHeader chunk;
             
           private:
             char format[4];
             
           public:
             std::string getFormat() const noexcept
               {
                  return std::string(static_cast<const char*>(format), sizeof(format));
               }
          };
        
        struct FmtHeader
          {
           public:
             ChunkHeader chunk;
             
           private:
             u16le audioFormat;
             u16le numChannels;
             u32le sampleRate;
             u32le byteRate;
             u16le blockAlign;
             u16le bitsPerSample;
             
           public:
             WavAudioFormat getAudioFormat() const noexcept
               {
                  u16 value = static_cast<u16>(audioFormat);
                  return static_cast<WavAudioFormat>(value);
               }
             
             u16 getNumChannels() const noexcept
               {
                  return static_cast<u16>(numChannels);
               }
             
             u32 getSampleRate() const noexcept
               {
                  return static_cast<u32>(sampleRate);
               }
             
             u16 getBitsPerSample() const noexcept
               {
                  return static_cast<u16>(bitsPerSample);
               }
          };
        
        struct FactHeader
          {
           public:
             ChunkHeader chunk;
             
           private:
             u32le sampleLength;
          };
        
        struct DispHeader
          {
           public:
             ChunkHeader chunk;
             
           private:
             u32le type;
             
           public:
             DispType getType() const noexcept
               {
                  u32 value = static_cast<u32>(type);
                  return static_cast<DispType>(value);
               }
          };
        
        struct ListHeader
          {
           public:
             ChunkHeader chunk;
             
           private:
             char listTypeID[4];
             
           public:
             std::string getListTypeID() const noexcept
               {
                  return std::string(static_cast<const char*>(listTypeID), sizeof(listTypeID));
               }
          };
#pragma pack(pop)
        
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
             mods::utils::RBuffer<FmtHeader> readFMT(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
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
             
             void buildInfo(int bitsPerSample, int nbChannels, int frequency, const std::string& description);
             
             
             WavConverter::ptr _converter;
             const mods::utils::RBuffer<u8> _fileBuffer;
             std::string _info;
             StatCollector::sptr _statCollector;
             size_t _length;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_WAVREADER_HPP