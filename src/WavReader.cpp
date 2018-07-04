
#include "WavReader.hpp"
#include "FileUtils.hpp"

#include <iostream>

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
        checkInit(std::equal(RIFF.begin(), RIFF.end(), 
                             _headerBuffer->chunkID, _headerBuffer->chunkID + sizeof(_headerBuffer->chunkID)), "Not a RIFF file");
        checkInit(_fileBuffer.size() >= _headerBuffer->getChunkSize() - sizeof(ChunkHeader), "RIFF chunk not complete");
        
        auto riffHeader = _fileBuffer.slice<RiffHeader>(0, 1);
        
        checkInit(std::equal(WAVE.begin(), WAVE.end(),
                             riffHeader->format, riffHeader->format + sizeof(riffHeader->format)), "Not a WAVE file");
        
        auto riffBuffer = _fileBuffer.slice<u8>(sizeof(RiffHeader), riffHeader->chunk.getChunkSize() - sizeof(riffHeader->format));
        
        size_t offset = 0;
        while(offset < riffBuffer.size())
          {
             /*auto chunkHeader = _fileBuffer.slice<ChunkHeader>(offset, 1);
             RBuffer<ChunkHeader>(_fileBuffer.slice<ChunkHeader>(*/
             std::cout << "TODO: WavReader: read chunk" << std::endl;
          }
        
        /*checkInit(std::equal(FMT.begin(), FMT.end(),
                             _headerBuffer->subchunkID, _headerBuffer->subchunkID + sizeof(_headerBuffer->subchunkID)));
        checkInit(_headerBuffer->getChunkSize() >= _headerBuffer->getSubchunkSize() + 20);
        
        check pcm;
        
        check data;
        check data size;*/
        
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
