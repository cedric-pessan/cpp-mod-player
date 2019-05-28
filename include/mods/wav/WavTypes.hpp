#ifndef MODS_WAV_WAVTYPES_HPP
#define MODS_WAV_WAVTYPES_HPP

#include "mods/utils/types.hpp"

namespace mods
{
   namespace wav
     {
        enum struct WavAudioFormat : u16
          {
             PCM = 0x0001,
               A_LAW = 0x0006,
               MU_LAW = 0x007,
               EXTENSIBLE = 0xFFFE
          };
        
        const std::string& toString(WavAudioFormat fmt);
        
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
             
             u16 getAudioFormatAsNumber() const noexcept
               {
                  return static_cast<u16>(audioFormat);
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
        
        struct ExtendedFmtHeader
          {
           public:
             FmtHeader fmt;
             
           private:
             u16le extensionSize;
             
           public:
             u16 getExtensionSize() const noexcept
               {
                  return static_cast<u16>(extensionSize);
               }
          };
        
        struct ExtensibleHeader
          {
           public:
             ExtendedFmtHeader extendedFmt;
             
           private:
             u16le validBitsPerSample;
             u32le channelMask;
             u16le audioFormat;
             u8 subformat[14];
             
           public:
             WavAudioFormat getAudioFormat() const noexcept
               {
                  u16 value = static_cast<u16>(audioFormat);
                  return static_cast<WavAudioFormat>(value);
               }
             
             u16 getValidBitsPerSample() const noexcept
               {
                  return static_cast<u16>(validBitsPerSample);
               }
             
             u32 getChannelMask() const noexcept
               {
                  return static_cast<u32>(channelMask);
               }
          };
        
        struct FactHeader
          {
           public:
             ChunkHeader chunk;
             
           private:
             u32le sampleLength;
          };
        
        struct AfspHeader
          {
           public:
             ChunkHeader chunk;
             
           private:
             char afspId[4];
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
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_WAVTYPES_HPP