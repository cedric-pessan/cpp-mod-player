
#include "WavReader.hpp"
#include "FileUtils.hpp"

#include <iostream>

namespace mods
{
   
   WavReader::WavReader()
     : _fileBuffer(FileUtils::mapFile()),
     _headerBuffer(_fileBuffer.slice<WavHeader>())
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
