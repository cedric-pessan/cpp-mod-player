
#include "WavReader.hpp"
#include "FileUtils.hpp"

#include <iostream>
#include <sstream>

namespace mods
{
   namespace
     {
        const std::string RIFF = "RIFF";
        const std::string WAVE = "WAVE";
        const std::string FMT = "fmt ";
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
        
        size_t offset = 0;
        while(offset < riffBuffer.size())
          {
             auto chunkHeader = riffBuffer.slice<ChunkHeader>(offset, 1);
             
             if(chunkHeader->getChunkID() == FMT)
               {
                  checkInit(chunkHeader->getChunkSize() <= riffBuffer.size() - offset &&
                            chunkHeader->getChunkSize() >= sizeof(FmtHeader) - sizeof(ChunkHeader) , "Incomplete FMT chunk");
                  
                  auto fmtHeader = riffBuffer.slice<FmtHeader>(offset, 1);
                  
                  checkInit(fmtHeader->getAudioFormat() == WavAudioFormat::PCM, "Only PCM is supported at the moment");
                  
                  checkInit(fmtHeader->chunk.getChunkSize() == sizeof(FmtHeader) - sizeof(ChunkHeader), "Extra fmt infos not yet implemented");
                  
                  std::cout << "TODO: fmt chunk" << std::endl;
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
   
   bool WavReader::isFinished() const
     {
        return _converter->isFinished();
     }
   
}
