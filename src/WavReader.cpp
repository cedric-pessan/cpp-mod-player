
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
             std::stringstream ss;
             ss << "Unknown RIFF chunk: " << chunkHeader->getChunkID();
             checkInit(false, ss.str());
          }
        
        /*checkInit(std::equal(FMT.begin(), FMT.end(),
                             _headerBuffer->subchunkID, _headerBuffer->subchunkID + sizeof(_headerBuffer->subchunkID)));
        checkInit(_headerBuffer->getChunkSize() >= _headerBuffer->getSubchunkSize() + 20);
        
        check pcm;
        
        check data;
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
