
#include "mods/wav/ALawConverter.hpp"
#include "mods/wav/ChannelCopyWavConverter.hpp"
#include "mods/wav/DVIADPCMDecoderConverter.hpp"
#include "mods/wav/DivideConverter.hpp"
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
#include "mods/wav/TruspeechDecoderConverter.hpp"
#include "mods/wav/UnpackToTypeConverter.hpp"
#include "mods/wav/UnsignedToSignedWavConverter.hpp"
#include "mods/wav/UpscaleWavConverter.hpp"
#include "mods/wav/WavConverter.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace mods
{
   namespace wav
     {
        namespace
          {
             constexpr std::array<int, 5> PCMBitsPerSample
               {
                  8, 12, 16, 24, 32
               };
          } // namespace
        
        // static
        auto WavConverter::buildConverter(const mods::utils::RBuffer<u8>& buffer, int bitsPerSample, int bitsPerContainer, int nbChannels, int frequency, const StatCollector::sptr& statCollector, WavAudioFormat codec, u32 channelMask,
                                          double peak) -> WavConverter::ptr
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
             switch(codec)
               {
                case WavAudioFormat::PCM:
                  if(std::find(PCMBitsPerSample.begin(), PCMBitsPerSample.end(), bitsPerSample) == PCMBitsPerSample.end())
                    {
                       std::cout << "invalid bits per sample for PCM: " << bitsPerSample << std::endl;
                    }
                  if(bitsPerSample == 8)
                    {
                       defaultValue = 128;
                    }
                  break;
                  
                case WavAudioFormat::A_LAW:
                  defaultValue = ALawConverter::getZero();
                  break;
                  
                case WavAudioFormat::MU_LAW:
                  defaultValue = MuLawConverter::getZero();
                  break;
                  
                case WavAudioFormat::DVI_ADPCM:
                case WavAudioFormat::GSM:
                case WavAudioFormat::IEEE_FLOAT:
                case WavAudioFormat::TRUSPEECH:
                  break;
                  
                default:
                  std::cout << "WavConverter: unknown default value for " << toString(codec) << std::endl;
               }
             
             std::vector<WavConverter::ptr> channels;
             for(int i=0; i<nbChannels; ++i)
               {
                  channels.push_back(std::make_unique<ReaderWavConverter>(buffer, defaultValue, statCollector, i, nbChannels, bitsPerContainer));
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
                  
                case 256:
                  if(codec != WavAudioFormat::TRUSPEECH)
                    {
                       std::cout << "WavConverter:: 256 container only supported with TRUSPEECH" << std::endl;
                    }
                  unpackedBitsPerContainer = bitsPerContainer = 16;
                  bitsPerSample = 16;
                  for(int i=0; i < nbChannels; ++i)
                    {
                       unpackedContainerChannels.push_back(std::make_unique<TruspeechDecoderConverter>(std::move(channels[i])));
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
		  
		case 4096:
		  if(codec != WavAudioFormat::DVI_ADPCM)
		    {
		       std::cout << "WavConverter: 4096 container should be DVI/ADPCM codec" << std::endl;
		    }
		  unpackedBitsPerContainer = bitsPerContainer = 16;
		  bitsPerSample = 16;
		  for(int i=0; i < nbChannels; ++i)
		    {
		       unpackedContainerChannels.push_back(std::make_unique<DVIADPCMDecoderConverter>(std::move(channels[i])));
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
                  if(isResamplableByPositiveIntegerFactor(frequency) && peak == 1.0)
                    {
                       upscaledBitsPerSample = 16;
                       switch(codec)
                         {
			  case WavAudioFormat::A_LAW:
			    for(int i = 0; i < nbChannels; ++i)
			      {
				 upscaledChannels.push_back(std::make_unique<ALawConverter>(std::move(unpackedSampleChannels[i])));
			      }
			    break;
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
                  if(isResamplableByPositiveIntegerFactor(frequency) && peak == 1.0)
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
                  if(isResamplableByPositiveIntegerFactor(frequency) && peak == 1.0)
                    {
                       switch(codec)
                         {
                          case WavAudioFormat::PCM:
                            for(int i = 0; i < nbChannels; ++i)
                              {
                                 upscaledChannels.push_back(std::move(unpackedSampleChannels[i]));
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
                  break;
                  
                default:
                  std::cout << "WavConverter: unsupported bits per sample to upscale: " << unpackedBitsPerSample << std::endl;
               }
             
             if(peak != 1.0)
               {
                  std::vector<WavConverter::ptr> peakVec = std::move(upscaledChannels);
                  upscaledChannels.clear();
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       upscaledChannels.emplace_back(std::make_unique<DivideConverter>(std::move(peakVec[i]), peak));
                    }
               }
             
             int resampledBitsPerSample = upscaledBitsPerSample;
             std::vector<WavConverter::ptr> resampledChannels;
             switch(frequency)
               {
                case 8000:
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       using ParamType = StaticResampleParameters<StandardFrequency::_8000, StandardFrequency::_44100>;
                       ParamType params;
                       resampledChannels.push_back(std::make_unique<ResampleConverter<ParamType>>(std::move(upscaledChannels[i]), params));
                    }
                  break;
		  
		case 10000:
		  for(int i = 0; i < nbChannels; ++i)
		    {
                       using ParamType = StaticResampleParameters<StandardFrequency::_10000, StandardFrequency::_44100>;
                       ParamType params;
		       resampledChannels.push_back(std::make_unique<ResampleConverter<ParamType>>(std::move(upscaledChannels[i]), params));
		    }
		  break;
		  
		case 11025:
		  for(int i = 0; i < nbChannels; ++i)
		    {
		       resampledChannels.push_back(buildResamplePositiveIntegerFactor<4>(resampledBitsPerSample, std::move(upscaledChannels[i])));
		    }
		  break;
                  
                case 22000:
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       using ParamType = StaticResampleParameters<StandardFrequency::_22000, StandardFrequency::_44100>;
                       ParamType params;
                       resampledChannels.push_back(std::make_unique<ResampleConverter<ParamType>>(std::move(upscaledChannels[i]), params));
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
                  
                case 48000:
                  for(int i = 0; i < nbChannels; ++i)
                    {
                       using ParamType = StaticResampleParameters<StandardFrequency::_48000, StandardFrequency::_44100>;
                       ParamType params;
                       resampledChannels.push_back(std::make_unique<ResampleConverter<ParamType>>(std::move(upscaledChannels[i]), params));
                    }
                  break;
                  
                default:
                  std::cout << "WARNING: WavConverter: non standard frequency: " << frequency << ", using generic resampler" << std::endl;
		  for(int i = 0; i < nbChannels; ++i)
		    {
                       using ParamType = DynamicResampleParameters;
                       ParamType params(frequency, 44100);
		       resampledChannels.push_back(std::make_unique<ResampleConverter<ParamType>>(std::move(upscaledChannels[i]), params));
		    }
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
                            floatChannels.push_back(std::make_unique<ToDoubleConverter<s16>>(std::move(resampledChannels[i])));
                         }
                       break;
                     case -1:
                       for(int i=0; i<nbChannels; ++i)
                         {
                            floatChannels.push_back(std::move(resampledChannels[i]));
                         }
                       break;
                     default:
                       std::cout << "WavConverter: unsupported bits per sample in surround mixer:" << resampledBitsPerSample << std::endl;
                    }
                  resampledBitsPerSample = -1;
                  
                  auto mixer = std::make_unique<MultiChannelMixer>(std::move(floatChannels), channelMask);
                  mixedRight = mixer->getRightChannel();
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
                  case -1:
                    return std::make_unique<ResamplePositiveIntegerFactor<double, FACTOR>>(std::move(src));
                  default:
                    std::cout << "WavConverter: unsupported bits per sample for resampling with positive integer factor: " << bitsPerSample << std::endl;
                    return nullptr;
                 }
            }
        
        // static
        bool WavConverter::isResamplableByPositiveIntegerFactor(int frequency)
          {
             return frequency == 44100 || frequency == 22050 || frequency == 11025;
          }
        
     } // namespace wav
} // namespace mods
