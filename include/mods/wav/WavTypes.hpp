#ifndef MODS_WAV_WAVTYPES_HPP
#define MODS_WAV_WAVTYPES_HPP

#include "mods/utils/PackedArray.hpp"
#include "mods/utils/types.hpp"

#include <string>

namespace mods
{
   namespace wav
     {
        enum struct WavAudioFormat : u16
          {
             PCM = 0x0001,
               ADPCM = 0x0002,
               IEEE_FLOAT = 0x0003,
               A_LAW = 0x0006,
               MU_LAW = 0x0007,
               OKI_ADPCM = 0x0010,
	       DVI_ADPCM = 0x0011,
               TRUSPEECH = 0x0022,
               GSM = 0x0031,
               EXTENSIBLE = 0xFFFE
          };
        
        auto toString(WavAudioFormat fmt) -> const std::string&;
        
        enum struct DispType : u32 // NOLINT(performance-enum-size)
          {
             TEXT = 1
          };
        
#pragma pack(push, 1)
        struct ChunkHeader
          {
           private:
             mods::utils::PackedArray<char,4> chunkID;
             u32le chunkSize;
             
           public:
             auto getChunkID() const noexcept -> std::string
               {
                  return { chunkID.data(), sizeof(chunkID) };
               }
             
             auto getChunkSize() const noexcept -> u32
               {
                  return static_cast<u32>(chunkSize);
               }
          };
        
        struct RiffHeader
          {
           private:
             ChunkHeader chunk;
             mods::utils::PackedArray<char,4> format;
             
           public:
             auto getChunkHeader() const noexcept -> const ChunkHeader&
               {
                  return chunk;
               }
             
             auto getFormat() const noexcept -> std::string
               {
                  return { format.data(), sizeof(format) };
               }
          };
        
        struct FmtHeader
          {
           private:
             ChunkHeader chunk;
             u16le audioFormat;
             u16le numChannels;
             u32le sampleRate;
             u32le byteRate;
             u16le blockAlign;
             u16le bitsPerSample;
             
           public:
             auto getChunkHeader() const noexcept -> const ChunkHeader&
               {
                  return chunk;
               }
             
             auto getAudioFormat() const noexcept -> WavAudioFormat
               {
                  const u16 value = static_cast<u16>(audioFormat);
                  return static_cast<WavAudioFormat>(value);
               }
             
             auto getAudioFormatAsNumber() const noexcept -> u16
               {
                  return static_cast<u16>(audioFormat);
               }
             
             auto getNumChannels() const noexcept -> u16
               {
                  return static_cast<u16>(numChannels);
               }
             
             auto getSampleRate() const noexcept -> u32
               {
                  return static_cast<u32>(sampleRate);
               }
             
             auto getBlockAlign() const noexcept -> u16
               {
                  return static_cast<u16>(blockAlign);
               }
             
             auto getBitsPerSample() const noexcept -> u16
               {
                  return static_cast<u16>(bitsPerSample);
               }
          };
        
        struct ExtendedFmtHeader
          {
           private:
             FmtHeader fmt;
             u16le extensionSize;
             
           public:
             auto getExtensionSize() const noexcept -> u16
               {
                  return static_cast<u16>(extensionSize);
               }
          };
        
        struct ExtensibleHeader
          {
           private:
             static constexpr int SUBFORMAT_LENGTH = 14;
             
             ExtendedFmtHeader extendedFmt;
             u16le validBitsPerSample;
             u32le channelMask;
             u16le audioFormat;
             mods::utils::PackedArray<u8, SUBFORMAT_LENGTH> subformat;
             
           public:
             auto getAudioFormat() const noexcept -> WavAudioFormat
               {
                  const u16 value = static_cast<u16>(audioFormat);
                  return static_cast<WavAudioFormat>(value);
               }
             
             auto getAudioFormatAsNumber() const noexcept -> u16
               {
                  return static_cast<u16>(audioFormat);
               }
             
             auto getValidBitsPerSample() const noexcept -> u16
               {
                  return static_cast<u16>(validBitsPerSample);
               }
             
             auto getChannelMask() const noexcept -> u32
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
             mods::utils::PackedArray<char, 4> afspId;
          };
        
        struct DispHeader
          {
           private:
             ChunkHeader chunk;
             u32le type;
             
           public:
             auto getType() const noexcept -> DispType
               {
                  const u32 value = static_cast<u32>(type);
                  return static_cast<DispType>(value);
               }
          };
        
        struct ListHeader
          {
           private:
             ChunkHeader chunk;
             mods::utils::PackedArray<char, 4> listTypeID;
             
           public:
             auto getChunkHeader() const noexcept -> const ChunkHeader&
               {
                  return chunk;
               }
             
             auto getListTypeID() const noexcept -> std::string
               {
                  return {listTypeID.data(), sizeof(listTypeID)};
               }
          };
        
        struct PeakHeader
          {
           public:
             ChunkHeader chunk;
             
           private:
             u32le version;
             u32le timestamp;
          };
        
        struct PPeakHeader
          {
             float value;
             u32le position;
          };
#pragma pack(pop)
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_WAVTYPES_HPP