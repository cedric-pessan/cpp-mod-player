
#include "WavReader.hpp"

#include <iostream>

namespace mods
{
   
   WavReader::WavReader()
     {
        _converter = std::make_unique<WavConverter>();
     }
   
   WavReader::~WavReader()
     {
     }
   
   bool WavReader::isFinished() const
     {
        return _converter->isFinished();
     }
   
}
