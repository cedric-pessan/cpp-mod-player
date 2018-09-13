
#include "mods/wav/DummyWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        DummyWavConverter::DummyWavConverter(WavConverter::ptr src)
          : _src(std::move(src))
            {
            }
        
        bool DummyWavConverter::isFinished() const
          {
             return _src->isFinished();
          }
     } //namespace wav
} // namespace mods
