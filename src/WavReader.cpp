
#include "WavReader.hpp"
#include "FileUtils.hpp"

#include <iostream>

namespace mods
{
   
   WavReader::WavReader(const std::string& filename)
     : _fileBuffer(FileUtils::mapFile(filename)),
     _headerBuffer(_fileBuffer.slice<WavHeader>(0, 1))
     {
        _converter = WavConverter::buildConverter(_headerBuffer->bitsPerSample);
     }
   
   WavReader::~WavReader()
     {
     }
   
   bool WavReader::isFinished() const
     {
        return _converter->isFinished();
     }
   
}
