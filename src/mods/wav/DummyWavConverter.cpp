
#include "mods/wav/DummyWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        bool DummyWavConverter::isFinished() const
          {
             std::cout << "TODO: DummyWavConverter::isFinished() const" << std::endl;
             return false;
          }
     } //namespace wav
} // namespace mods
