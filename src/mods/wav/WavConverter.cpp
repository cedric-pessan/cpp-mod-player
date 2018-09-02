
#include "mods/wav/WavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        // static
        WavConverter::ptr WavConverter::buildConverter(/*int bitsPerSample*/)
          {
             /* pipeline:
              sampleReader
              demux
              upscale or dummy
              resample or csoimple resple or dummy
              mix or dummy
              downscale or dummy
              mux
             */
               
             
             std::cout << "TODO: WavConverter::buildConverter()" << std::endl;
             return nullptr;
          }
     } // namespace wav
} // namespace mods
