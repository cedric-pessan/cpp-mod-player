
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
     : _fileBuffer(FileUtils::mapFile(filename))/*,
     _headerBuffer(_fileBuffer.slice<WavHeader>(0, 1))*/
     {
        size_t offset = 0;
        while(offset < _fileBuffer.size())
          {
             std::cout << "TODO: WavReader: read chunk" << std::endl;
          }
        
        /*checkInit(std::equal(RIFF.begin(), RIFF.end(), 
                             _headerBuffer->chunkID, _headerBuffer->chunkID + sizeof(_headerBuffer->chunkID)));
        checkInit(_fileBuffer.size() >= _headerBuffer->getChunkSize() - 8);
        checkInit(std::equal(WAVE.begin(), WAVE.end(),
                             _headerBuffer->format, _headerBuffer->format + sizeof(_headerBuffer->format)));
        
        checkInit(std::equal(FMT.begin(), FMT.end(),
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
