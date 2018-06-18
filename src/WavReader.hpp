#ifndef _WAVREADER_HPP_
#define _WAVREADER_HPP_

#include "ModuleReader.hpp"
#include "WavConverter.hpp"
#include "RBuffer.hpp"
#include "types.hpp"

namespace mods
{
#pragma pack(push, 1)
   struct WavHeader
     {
        char chunkID[4];
        u32 chunkSize;
        char format[4];
        
        char subchunkID[4];
        u32 subchunkSize;
        u16 audioFormat;
        u16 numChannels;
        u32 sampleRate;
        u32 byteRate;
        u16 blockAlign;
        u16 bitsPerSample;
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
        RBuffer<WavHeader> _headerBuffer;
     };
   
}

#endif // _WAVREADER_HPP_