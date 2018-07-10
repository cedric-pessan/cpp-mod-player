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
   
   struct WavHeader
     {
      private:
        char subchunkID[4];
        u32 subchunkSize;
        u16 audioFormat;
        u16 numChannels;
        u32 sampleRate;
        u32 byteRate;
        u16 blockAlign;
        u16 bitsPerSample;
        
        u32 getSubchunkSize() const noexcept;
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