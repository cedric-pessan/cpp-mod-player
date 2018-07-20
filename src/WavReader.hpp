#ifndef _WAVREADER_HPP_
#define _WAVREADER_HPP_

#include "ModuleReader.hpp"
#include "WavConverter.hpp"
#include "RBuffer.hpp"
#include "types.hpp"

namespace mods
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
        
        RBuffer<FmtHeader> readFMT(const RBuffer<ChunkHeader>& chunkHeader,
                                   const RBuffer<u8>& riffBuffer,
                                   size_t offset) const;
        RBuffer<FactHeader> readFact(const RBuffer<ChunkHeader>& chunkHeader,
                                     const RBuffer<u8>& riffBuffer,
                                     size_t offset) const;
        RBuffer<u8> readData(const RBuffer<ChunkHeader>& chunkHeader,
                             const RBuffer<u8>& riffBuffer,
                             size_t offset) const;
        
        
        WavConverter::ptr _converter;
        RBuffer<u8> _fileBuffer;
        RBuffer<ChunkHeader> _headerBuffer;
     };
   
}

#endif // _WAVREADER_HPP_