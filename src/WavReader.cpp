
#include "WavReader.hpp"
#include "FileUtils.hpp"
#include "optional.hpp"

#include <iostream>
#include <sstream>

namespace mods
{
   namespace
     {
        const std::string RIFF = "RIFF";
        const std::string WAVE = "WAVE";
        const std::string FMT = "fmt ";
        const std::string FACT = "fact";
     }
   
   WavReader::WavReader(const std::string& filename)
     : _fileBuffer(FileUtils::mapFile(filename)),
     _headerBuffer(_fileBuffer.slice<ChunkHeader>(0, 1))
     {
        checkInit(_headerBuffer->getChunkID() == RIFF, "Not a RIFF file");
        checkInit(_fileBuffer.size() >= _headerBuffer->getChunkSize() - sizeof(ChunkHeader), "RIFF chunk not complete");
        
        auto riffHeader = _fileBuffer.slice<RiffHeader>(0, 1);
        
        checkInit(riffHeader->getFormat() == WAVE, "Not a WAVE file");
        
        auto riffBuffer = _fileBuffer.slice<u8>(sizeof(RiffHeader), riffHeader->chunk.getChunkSize() - sizeof(RiffHeader) + sizeof(ChunkHeader));
        
        optional<RBuffer<FmtHeader>> optFmtHeader;
        optional<RBuffer<FactHeader>> optFactHeader;
        
        size_t offset = 0;
        while(offset < riffBuffer.size())
          {
             auto chunkHeader = riffBuffer.slice<ChunkHeader>(offset, 1);
             
             if(chunkHeader->getChunkID() == FMT)
               {
                  checkInit(!optFmtHeader.has_value(), "Multiple fmt chunks defined");
                  optFmtHeader = readFMT(chunkHeader, riffBuffer, offset);
               }
             else if(chunkHeader->getChunkID() == FACT)
               {
                  checkInit(!optFactHeader.has_value(), "Multiple fact chunks defined");
                  optFactHeader = readFact(chunkHeader, riffBuffer, offset);
               }
             else
               {
                  std::stringstream ss;
                  ss << "Unknown RIFF chunk: " << chunkHeader->getChunkID();
                  checkInit(false, ss.str());
               }
             
             offset += chunkHeader->getChunkSize() + sizeof(ChunkHeader);
          }
        
        /*check data;
        check data size;*/
        
        std::cout << "TODO: WavReader: everything is parsed" << std::endl;
        //_converter = WavConverter::buildConverter(_headerBuffer->bitsPerSample);
     }
   
   WavReader::~WavReader()
     {
     }
   
   RBuffer<FmtHeader> WavReader::readFMT(const RBuffer<ChunkHeader>& chunkHeader,
                                         const RBuffer<u8>& riffBuffer,
                                         size_t offset) const
     {
        checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                  chunkHeader->getChunkSize() >= sizeof(FmtHeader) - sizeof(ChunkHeader) , "Incomplete FMT chunk");
        
        auto fmtHeader = riffBuffer.slice<FmtHeader>(offset, 1);
        
        checkInit(fmtHeader->getAudioFormat() == WavAudioFormat::PCM, "Only PCM is supported at the moment");
        
        checkInit(fmtHeader->chunk.getChunkSize() == sizeof(FmtHeader) - sizeof(ChunkHeader), "Extra fmt infos not yet implemented");
        
        return fmtHeader;
     }
   
   RBuffer<FactHeader> WavReader::readFact(const RBuffer<ChunkHeader>& chunkHeader,
                                           const RBuffer<u8>& riffBuffer,
                                           size_t offset) const
     {
        checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset - sizeof(ChunkHeader) &&
                  chunkHeader->getChunkSize() == sizeof(FactHeader) - sizeof(ChunkHeader) , "Incomplete Fact chunk");
        
        auto factHeader = riffBuffer.slice<FactHeader>(offset, 1);
        
        return factHeader;
     }
   
   bool WavReader::isFinished() const
     {
        return _converter->isFinished();
     }
   
}
