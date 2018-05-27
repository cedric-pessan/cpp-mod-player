
#include "WavReader.hpp"

#include <iostream>

namespace mods
{
   
   WavReader::WavReader()
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
