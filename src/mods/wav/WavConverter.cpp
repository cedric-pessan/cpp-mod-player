
#include "mods/wav/DummyWavConverter.hpp"
#include "mods/wav/WavConverter.hpp"
#include "mods/wav/MultiplexerWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        // static
        WavConverter::ptr WavConverter::buildConverter(int bitsPerSample)
          {
             /* pipeline:
              sampleReader
              demux
              upscale or dummy
              resample or simple resple or dummy
              mix or dummy
              downscale or dummy
              mux
             */
             
             int upscaledBitsPerSample = bitsPerSample;
             
             switch(bitsPerSample)
               {
                case 8:
                  upscaledBitsPerSample = 16;
                  break;
                default:
                  std::cout << "unsupported bitsPerSample to upscale:" << bitsPerSample << std::endl;
               }
             
             ptr downScaledLeft;
             ptr downScaledRight;
             
             switch(upscaledBitsPerSample)
               {
                case 16:
                  downScaledLeft = std::make_unique<DummyWavConverter>();
                  downScaledRight = std::make_unique<DummyWavConverter>();
                  break;
                default:
                  std::cout << "unsupported bitsPerSample to downscale:" << upscaledBitsPerSample << std::endl;
               }
                    
             auto mux = std::make_unique<MultiplexerWavConverter>(std::move(downScaledLeft), std::move(downScaledRight));
             return mux;
          }
     } // namespace wav
} // namespace mods
