
#include "mods/wav/UpscaleWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        UpscaleWavConverter::UpscaleWavConverter(WavConverter::ptr src)
          : _src(std::move(src))
            {
            }
        
        bool UpscaleWavConverter::isFinished() const
          {
             return _src->isFinished();
          }
     } // namespace wav
} // namespace mods
