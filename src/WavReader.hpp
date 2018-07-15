#ifndef _WAVREADER_HPP_
#define _WAVREADER_HPP_

#include "ModuleReader.hpp"
#include "WavConverter.hpp"
#include "RBuffer.hpp"
#include "types.hpp"

namespace mods
{
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
        
        WavConverter::ptr _converter;
        RBuffer<u8> _fileBuffer;
        RBuffer<ChunkHeader> _headerBuffer;
     };
   
}

#endif // _WAVREADER_HPP_