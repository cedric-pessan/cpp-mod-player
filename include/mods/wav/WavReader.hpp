#ifndef _WAVREADER_HPP_
#define _WAVREADER_HPP_

#include "mods/ModuleReader.hpp"
#include "WavConverter.hpp"
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
        
#pragma pack(push, 1)
        struct ChunkHeader
          {
           private:
             char chunkID[4];
             u32le chunkSize;
             
           public:
             std::string getChunkID() const noexcept
               {
                  return std::string(chunkID, sizeof(chunkID));
               }
             
             u32 getChunkSize() const noexcept
               {
                  return chunkSize;
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
                  return std::string(format, sizeof(format));
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
                  u16 value = audioFormat;
                  return static_cast<WavAudioFormat>(value);
               }
          };
        
        struct FactHeader
          {
           public:
             ChunkHeader chunk;
             
           private:
             u32le sampleLength;
          };
#pragma pack(pop)
        
        class WavReader : public ModuleReader
          {
           public:
             explicit WavReader(const std::string& filename);
             virtual ~WavReader();
             
             virtual bool isFinished() const override;
             
           private:
             WavReader() = delete;
             WavReader(const WavReader&) = delete;
             WavReader& operator=(const WavReader&) = delete;
             
             mods::utils::RBuffer<FmtHeader> readFMT(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                                     const mods::utils::RBuffer<u8>& riffBuffer,
                                                     size_t offset) const;
             mods::utils::RBuffer<FactHeader> readFact(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                                       const mods::utils::RBuffer<u8>& riffBuffer,
                                                       size_t offset) const;
             mods::utils::RBuffer<u8> readData(const mods::utils::RBuffer<ChunkHeader>& chunkHeader,
                                               const mods::utils::RBuffer<u8>& riffBuffer,
                                               size_t offset) const;
             
             
             WavConverter::ptr _converter;
             mods::utils::RBuffer<u8> _fileBuffer;
             mods::utils::RBuffer<ChunkHeader> _headerBuffer;
          };
     }
}

#endif // _WAVREADER_HPP_