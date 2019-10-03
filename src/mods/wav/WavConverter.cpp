
#include "mods/wav/ALawConverter.hpp"
#include "mods/wav/ChannelCopyWavConverter.hpp"
#include "mods/wav/DownscaleWavConverter.hpp"
#include "mods/wav/FillLSBConverter.hpp"
#include "mods/wav/FromDoubleConverter.hpp"
#include "mods/wav/GSMDecoderConverter.hpp"
#include "mods/wav/MuLawConverter.hpp"
#include "mods/wav/MultiChannelMixer.hpp"
#include "mods/wav/MultiplexerWavConverter.hpp"
#include "mods/wav/ReaderWavConverter.hpp"
#include "mods/wav/ResampleConverter.hpp"
#include "mods/wav/ResamplePositiveIntegerFactor.hpp"
#include "mods/wav/ToDoubleConverter.hpp"
#include "mods/wav/UnpackToTypeConverter.hpp"
#include "mods/wav/UnsignedToSignedWavConverter.hpp"
#include "mods/wav/UpscaleWavConverter.hpp"
#include "mods/wav/WavConverter.hpp"

#include <iostream>
#include <vector>

namespace mods
{
   namespace wav
     {
        // static
        WavConverter::ptr WavConverter::buildConverter(const mods::utils::RBuffer<u8>& buffer, int bitsPerSample, int bitsPerContainer, int nbChannels, int frequency, StatCollector::sptr statCollector, WavAudioFormat codec, u32 channelMask)
          {
             /* pipeline:
              sampleReader
              demux
              unpack or dummy
              upscale to fit container or dummy
              upscale or dummy
              resample or simple resple or dummy
              mix or dummy
              downscale or dummy
              mux
             */
             
             u8 defaultValue = 0;
             switch(bitsPerSample)
               {
                case 0:
                  if(codec != WavAudioFormat::GSM)
                    {
                       std::cout << "0 bit value is only valid for GSM codec" << std::endl;
                    }
                  break;
                  
                case 8:
                  if(codec == WavAudioFormat::A_LAW)
                    {
                       defaultValue = ALawConverter::getZero();
                    }
                  else if(codec == WavAudioFormat::MU_LAW)
                    {
                       defaultValue = MuLawConverter::getZero();
                    }
                  else
                    {
                       defaultValue = 128;
                    }
                  break;
                  
                case 12:
                case 16:
                case 24:
                  if(codec != WavAudioFormat::PCM)
                    {
                       std::cout << "WavConverter: codec is not pcm and bits per sample is not 8" << std::endl;
                    }
                  break;
                  
                case 32:
                  if(codec != WavAudioFormat::PCM && codec != WavAudioFormat::IEEE_FLOAT)
                    {
                       std::cout << "WavConverter: 32 bits should be used with pcm or ieee float codec" << std::endl;
                    }
                  break;
                  
                case 64:
                  if(codec != WavAudioFormat::IEEE_FLOAT)
                    {
                       std::cout << "WavConverter: 64 bits should be used with pcm or ieee float codec" << std::endl;
                    }
                  break;
                  
                default:
                  std::cout << "WavConverter: unknown default value for " << bitsPerSample << " bits per sample" << std::endl;
               }
             
             std::vector<WavConverter::ptr> channels;
             switch(bitsPerContainer)
               {
                case 8:
                  buildDemuxStage<8>(&channels, nbChannels, defaultValue, buffer, std::move(statCollector));
                  break;
                  
                case 16:
                  buildDemuxStage<16>(&channels, nbChannels, defaultValue, buffer, std::move(statCollector));
                  break;
                  
                case 24:
                  buildDemuxStage<24>(&channels, nbChannels, defaultValue, buffer, std::move(statCollector));
                  break;
                  
                case 32:
                  buildDemuxStage<32>(&channels, nbChannels, defaultValue, buffer, std::move(statCollector));
                  break;
                  
                case 64:
                  buildDemuxStage<64>(&channels, nbChannels, defaultValue, buffer, std::move(statCollector));
                  break;
                  
                case 520:
                  buildDemuxStage<520>(&channels, nbChannels, defaultValue, buffer, std::move(statCollector));
                  break;
                  
                default:
                  std::cout << "WavConverter: unsupported " << bitsPerContainer << " bits per container for demux stage" << std::endl;
               }
             
             int unpackedBitsPerContainer = bitsPerContainer;
             std::vector<WavConverter::ptr> unpackedContainerChannels;
             switch(bitsPerContainer)
               {
                case 8:
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       unpackedContainerChannels.push_back(std::move(channels[i]));
                    }
                  break;
                  
                case 16:
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       unpackedContainerChannels.push_back(std::move(channels[i]));
                    }
                  break;
                  
                case 24:
                  unpackedBitsPerContainer = 32;
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       unpackedContainerChannels.push_back(std::make_unique<UnpackToTypeConverter<s32>>(std::move(channels[i]), 3));
                    }
                  break;
                  
                case 32:
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       unpackedContainerChannels.push_back(std::move(channels[i]));
                    }
                  break;
                  
                case 64:
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       unpackedContainerChannels.push_back(std::move(channels[i]));
                    }
                  break;
                  
                case 520:
                  if(codec != WavAudioFormat::GSM)
                    {
                       std::cout << "WavConverter: 520 container should be GSM codec" << std::endl;
                    }
                  unpackedBitsPerContainer = bitsPerContainer = 16;
                  bitsPerSample = 13;
                  for(int i=0; i < nbChannels; ++i)
                    {
                       unpackedContainerChannels.push_back(std::make_unique<GSMDecoderConverter>(std::move(channels[i])));
                    }
                  break;
                  
                default:
                  std::cout << "WavConverter: unsupported " << bitsPerContainer << " bits per container for unpacking container stage" << std::endl;
               }
             
             int unpackedBitsPerSample = unpackedBitsPerContainer;
             std::vector<WavConverter::ptr> unpackedSampleChannels;
             if(unpackedBitsPerContainer == bitsPerSample)
               {
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       unpackedSampleChannels.push_back(std::move(unpackedContainerChannels[i]));
                    }
               }
             else
               {
                  switch(unpackedBitsPerContainer)
                    {
                     case 16:
                       for(int i = 0; i < nbChannels; ++i)
                         {
                            unpackedSampleChannels.push_back(std::make_unique<FillLSBConverter<s16>>(std::move(unpackedContainerChannels[i]), unpackedBitsPerContainer - bitsPerSample));
                         }
                       break;
                       
                     case 32:
                       for(int i = 0; i < nbChannels; ++i)
                         {
                            unpackedSampleChannels.push_back(std::make_unique<FillLSBConverter<s32>>(std::move(unpackedContainerChannels[i]), unpackedBitsPerContainer - bitsPerSample));
                         }
                       break;
                       
                     default:
                       std::cout << "WavConverter: unpacked bits per container is different than bits per sample (" << bitsPerSample << "/" << unpackedBitsPerContainer << ")" << std::endl;
                    }
               }
             
             int upscaledBitsPerSample = unpackedBitsPerSample;
             std::vector<WavConverter::ptr> upscaledChannels;
             switch(unpackedBitsPerSample)
               {
                case 8:
                  if(isResamplableByPositiveIntegerFactor(frequency)) 
                    {
                       upscaledBitsPerSample = 16;
                       switch(codec)
                         {
                          case WavAudioFormat::PCM:
                            for(int i = 0; i < nbChannels; ++i)
                              {
                                 auto signedChannel = std::make_unique<UnsignedToSignedWavConverter<u8>>(std::move(unpackedSampleChannels[i]));
                                 upscaledChannels.push_back(std::make_unique<UpscaleWavConverter<s16, s8>>(std::move(signedChannel)));
                              }
                            break;
                          default:
                            std::cout << "WavConverter: unsuported codec in upscale stage for 8 bits sample (integer factor resampling case)" << std::endl;
                         }
                    }
                  else
                    {
                       upscaledBitsPerSample = -1;
                       switch(codec)
                         {
                          case WavAudioFormat::A_LAW:
                            for(int i = 0; i < nbChannels; ++i)
                              {
                                 auto aLawChannel = std::make_unique<ALawConverter>(std::move(unpackedSampleChannels[i]));
                                 upscaledChannels.push_back(std::make_unique<ToDoubleConverter<s16>>(std::move(aLawChannel)));
                              }
                            break;
                          case WavAudioFormat::MU_LAW:
                            for(int i = 0; i < nbChannels; ++i)
                              {
                                 auto muLawChannel = std::make_unique<MuLawConverter>(std::move(unpackedSampleChannels[i]));
                                 upscaledChannels.push_back(std::make_unique<ToDoubleConverter<s16>>(std::move(muLawChannel)));
                              }
                            break;
                          case WavAudioFormat::PCM:
                            for(int i = 0; i < nbChannels; ++i)
                              {
                                 auto signedChannel = std::make_unique<UnsignedToSignedWavConverter<u8>>(std::move(unpackedSampleChannels[i]));
                                 upscaledChannels.push_back(std::make_unique<ToDoubleConverter<s8>>(std::move(signedChannel)));
                              }
                            break;
                          default:
                            std::cout << "WavConverter: unsuported codec in upscale stage for 8 bits sample" << std::endl;
                         }
                    }
                  break;
                  
                case 16:
                  if(isResamplableByPositiveIntegerFactor(frequency))
                    {
                       for(int i = 0; i < nbChannels; ++i)
                         {
                            upscaledChannels.push_back(std::move(unpackedSampleChannels[i]));
                         }
                    }
                  else
                    {
                       upscaledBitsPerSample = -1;
                       for(int i = 0; i < nbChannels; ++i)
                         {
                            upscaledChannels.push_back(std::make_unique<ToDoubleConverter<s16>>(std::move(unpackedSampleChannels[i])));
                         }
                    }
                  break;
                  
                case 32:
                  if(isResamplableByPositiveIntegerFactor(frequency))
                    {
                       switch(codec)
                         {
                          case WavAudioFormat::PCM:
                            for(int i = 0; i < nbChannels; ++i)
                              {
                                 upscaledChannels.push_back(std::move(unpackedSampleChannels[i]));
                              }
                            break;
                          default:
                            std::cout << "WavConverter: unsuported codec in upscale stage for 32 bits sample (integer factor resampling)" << std::endl;
                         }
                    }
                  else
                    {
                       switch(codec)
                         {
                          case WavAudioFormat::PCM:
                            upscaledBitsPerSample = -1;
                            for(int i = 0; i < nbChannels; ++i)
                              {
                                 upscaledChannels.push_back(std::make_unique<ToDoubleConverter<s32>>(std::move(unpackedSampleChannels[i])));
                              }
                            break;
                          case WavAudioFormat::IEEE_FLOAT:
                            upscaledBitsPerSample = -1;
                            for(int i = 0; i < nbChannels; ++i)
                              {
                                 upscaledChannels.push_back(std::make_unique<ToDoubleConverter<float>>(std::move(unpackedSampleChannels[i])));
                              }
                            break;
                          default:
                            std::cout << "WavConverter: unsuported codec in upscale stage for 32 bits sample" << std::endl;
                         }
                    }
                  break;
                  
                case 64:
                  if(codec != WavAudioFormat::IEEE_FLOAT)
                    {
                       std::cout << "WavConverter: unsuported codec in upscale stage for 64 bits sample" << std::endl;
                    }
                  else
                    {
                       upscaledBitsPerSample = -1;
                       for(int i = 0; i < nbChannels; ++i)
                         {
                            upscaledChannels.push_back(std::move(unpackedSampleChannels[i]));
                         }
                    }
                  
                default:
                  std::cout << "WavConverter: unsupported bits per sample to upscale: " << unpackedBitsPerSample << std::endl;
               }
             
             int resampledBitsPerSample = upscaledBitsPerSample;
             std::vector<WavConverter::ptr> resampledChannels;
             switch(frequency)
               {
                case 8000:
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       resampledChannels.push_back(std::make_unique<ResampleConverter<8000, 44100>>(std::move(upscaledChannels[i])));
                    }
                  break;
                  
                case 22000:
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       resampledChannels.push_back(std::make_unique<ResampleConverter<22000, 44100>>(std::move(upscaledChannels[i])));
                    }
                  break;
                  
                case 22050:
                  for(int i = 0; i < nbChannels; ++i) 
                    {
                       resampledChannels.push_back(buildResamplePositiveIntegerFactor<2>(resampledBitsPerSample, std::move(upscaledChannels[i])));
                    }
                  break;
                  
                case 44100:
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       resampledChannels.push_back(std::move(upscaledChannels[i]));
                    }
                  break;
                  
                default:
                  std::cout << "WavConverter: unsupported frequency: " << frequency << std::endl;
               }
             
             ptr mixedLeft;
             ptr mixedRight;
             
             if(channelMask == 0)
               {
                  switch(nbChannels)
                    {
                     case 1:
                         {
                            auto duplicator = std::make_unique<ChannelCopyWavConverter>(std::move(resampledChannels[0]));
                            mixedRight = duplicator->getCopy();
                            mixedLeft = std::move(duplicator);
                         }
                       break;
                       
                     case 2:                  
                       mixedLeft = std::move(resampledChannels[0]);
                       mixedRight = std::move(resampledChannels[1]);
                       break;
                       
                     default:
                       std::cout << "WavConverter: unsupported number of channels for mixing stage: " << nbChannels << std::endl;
                    }
               }
             else
               {
                  std::vector<WavConverter::ptr> floatChannels;
                  switch(resampledBitsPerSample)
                    {
                     case 16:
                       for(int i=0; i<nbChannels; ++i)
                         {
                            floatChannels.push_back(std::make_unique<ToDoubleConverter<float>>(std::move(resampledChannels[i])));
                         }
                       break;
                     default:
                       std::cout << "WavConverter: unsupported bits per sample in surround mixer:" << resampledBitsPerSample << std::endl;
                    }
                  resampledBitsPerSample = -1;
                  
                  auto mixer = std::make_unique<MultiChannelMixer>();
                  mixedRight = mixer->getRight();
                  mixedLeft = std::move(mixer);
               }
             
             ptr downScaledLeft;
             ptr downScaledRight;
             
             switch(resampledBitsPerSample)
               {
                case 16:
                  downScaledLeft = std::move(mixedLeft);
                  downScaledRight = std::move(mixedRight);
                  break;
                  
                case 32:
                  downScaledLeft = std::make_unique<DownscaleWavConverter<s16, s32>>(std::move(mixedLeft));
                  downScaledRight = std::make_unique<DownscaleWavConverter<s16, s32>>(std::move(mixedRight));
                  break;
                  
                case -1:
                  downScaledLeft = std::make_unique<FromDoubleConverter<s16>>(std::move(mixedLeft));
                  downScaledRight = std::make_unique<FromDoubleConverter<s16>>(std::move(mixedRight));
                  break;
                  
                default:
                  std::cout << "WavConverter: unsupported bits per sample to downscale: " << resampledBitsPerSample << std::endl;
               }
                    
             auto mux = std::make_unique<MultiplexerWavConverter>(std::move(downScaledLeft), std::move(downScaledRight));
             return mux;
          }
        
        // static
        template<int FACTOR>
          WavConverter::ptr WavConverter::buildResamplePositiveIntegerFactor(int bitsPerSample, WavConverter::ptr src)
            {
               switch(bitsPerSample)
                 {
                  case 16:
                    return std::make_unique<ResamplePositiveIntegerFactor<s16, FACTOR>>(std::move(src));
                  default:
                    std::cout << "WavConverter: unsupported bits per sample for resampling with positive integer factor: " << bitsPerSample << std::endl;
                    return nullptr;
                 }
            }
        
        // static
        template<int BITSPERCONTAINER>
          void WavConverter::buildDemuxStage(std::vector<WavConverter::ptr>* channels, int nbChannels, u8 defaultValue, const mods::utils::RBuffer<u8>& buffer, StatCollector::sptr statCollector)
            {
               switch(nbChannels)
                 {
                  case 1:
                    channels->push_back(std::make_unique<ReaderWavConverter<0,1,BITSPERCONTAINER>>(buffer, defaultValue, statCollector));
                    break;
                    
                  case 2:
                    channels->push_back(std::make_unique<ReaderWavConverter<0,2,BITSPERCONTAINER>>(buffer, defaultValue, statCollector));
                    channels->push_back(std::make_unique<ReaderWavConverter<1,2,BITSPERCONTAINER>>(buffer, defaultValue, statCollector));
                    break;
                    
                  case 6:
                    channels->push_back(std::make_unique<ReaderWavConverter<0,6,BITSPERCONTAINER>>(buffer, defaultValue, statCollector));
                    channels->push_back(std::make_unique<ReaderWavConverter<1,6,BITSPERCONTAINER>>(buffer, defaultValue, statCollector));
                    channels->push_back(std::make_unique<ReaderWavConverter<2,6,BITSPERCONTAINER>>(buffer, defaultValue, statCollector));
                    channels->push_back(std::make_unique<ReaderWavConverter<3,6,BITSPERCONTAINER>>(buffer, defaultValue, statCollector));
                    channels->push_back(std::make_unique<ReaderWavConverter<4,6,BITSPERCONTAINER>>(buffer, defaultValue, statCollector));
                    channels->push_back(std::make_unique<ReaderWavConverter<5,6,BITSPERCONTAINER>>(buffer, defaultValue, statCollector));
                    break;
                    
                  default:
                    std::cout << "WavConverter: unsupported number of channels for demux stage:" << nbChannels << std::endl;
                 }
            }
        
        // static
        bool WavConverter::isResamplableByPositiveIntegerFactor(int frequency)
          {
             return frequency == 44100 || frequency == 22050 || frequency == 11025;
          }
        
     } // namespace wav
} // namespace mods
