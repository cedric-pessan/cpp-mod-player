
#include "mods/wav/ChannelCopyWavConverter.hpp"
#include "mods/wav/DummyWavConverter.hpp"
#include "mods/wav/WavConverter.hpp"
#include "mods/wav/MultiplexerWavConverter.hpp"
#include "mods/wav/ResamplePositiveIntegerFactor.hpp"
#include "mods/wav/UpscaleWavConverter.hpp"

#include <iostream>
#include <vector>

namespace mods
{
   namespace wav
     {
        // static
        WavConverter::ptr WavConverter::buildConverter(int bitsPerSample, int nbChannels, int frequency)
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
             
             std::vector<WavConverter::ptr> channels;
             switch(nbChannels)
               {
                default:
                  std::cout << "WavConverter: unsupported number of channels for demux stage:" << nbChannels << std::endl;
               }
             
             int upscaledBitsPerSample = bitsPerSample;
             std::vector<WavConverter::ptr> upscaledChannels;
             switch(bitsPerSample)
               {
                case 8:
                  upscaledBitsPerSample = 16;
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       upscaledChannels.push_back(std::make_unique<UpscaleWavConverter>(std::move(channels[i])));
                    }
                  break;
                default:
                  std::cout << "WavConverter: unsupported bits per sample to upscale: " << bitsPerSample << std::endl;
               }
             
             int resampledBitsPerSample = upscaledBitsPerSample;
             std::vector<WavConverter::ptr> resampledChannels;
             switch(frequency)
               {
                case 22050:
                  for(int i = 0; i < nbChannels; ++i) 
                    {
                       resampledChannels.push_back(std::make_unique<ResamplePositiveIntegerFactor>(std::move(upscaledChannels[i])));
                    }
                  break;
                  
                default:
                  std::cout << "WavConverter: unsupported frequency: " << frequency << std::endl;
               }
             
             ptr mixedLeft;
             ptr mixedRight;
             
             switch(nbChannels)
               {
                case 1:
                    {
                       auto duplicator = std::make_unique<ChannelCopyWavConverter>(std::move(resampledChannels[0]));
                       mixedRight = duplicator->getCopy();
                       mixedLeft = std::move(duplicator);
                    }
                  break;
                default:
                  std::cout << "WavConverter: unsupported number of channels for mixing stage: " << nbChannels << std::endl;
               }
             
             ptr downScaledLeft;
             ptr downScaledRight;
             
             switch(resampledBitsPerSample)
               {
                case 16:
                  downScaledLeft = std::make_unique<DummyWavConverter>(std::move(mixedLeft));
                  downScaledRight = std::make_unique<DummyWavConverter>(std::move(mixedRight));
                  break;
                default:
                  std::cout << "WavConverter: unsupported bits per sample to downscale: " << resampledBitsPerSample << std::endl;
               }
                    
             auto mux = std::make_unique<MultiplexerWavConverter>(std::move(downScaledLeft), std::move(downScaledRight));
             return mux;
          }
     } // namespace wav
} // namespace mods
