
#include "mods/StandardFrequency.hpp"
#include "mods/utils/OpenCLManager.hpp"
#include "mods/wav/ADPCMDecoderConverter.hpp"
#include "mods/wav/ALawConverter.hpp"
#include "mods/wav/ChannelCopyWavConverter.hpp"
#include "mods/wav/DVIADPCMDecoderConverter.hpp"
#include "mods/wav/DemuxConverter.hpp"
#include "mods/wav/DivideConverter.hpp"
#include "mods/wav/DownscaleWavConverter.hpp"
#include "mods/wav/FillLSBConverter.hpp"
#include "mods/wav/FromDoubleConverter.hpp"
#include "mods/wav/GSMDecoderConverter.hpp"
#include "mods/wav/MuLawConverter.hpp"
#include "mods/wav/MultiChannelMixer.hpp"
#include "mods/wav/MultiplexerWavConverter.hpp"
#include "mods/wav/OKIADPCMDecoderConverter.hpp"
#include "mods/wav/OpenCLConverterTypes.hpp"
#include "mods/wav/ReaderWavConverter.hpp"
#include "mods/wav/ResampleParameters.hpp"
#include "mods/wav/ResamplePositiveIntegerFactor.hpp"
#include "mods/wav/SoftwareResampleConverter.hpp"
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
             
             constexpr int default8BitsPCMValue = 128;
             
             enum struct WavBitsPerContainer : int
               {
                  _8 = 8,
                    _16 = 16,
                    _24 = 24,
                    _32 = 32,
                    _64 = 64
               };
             
             enum struct WavBitsPerSample : int
               {
                  _8 = 8,
                    _16 = 16,
                    _32 = 32,
                    _64 = 64,
                    _double = -1
               };
             
             template<int FACTOR>
               auto buildResamplePositiveIntegerFactor(WavBitsPerSample bitsPerSample, WavConverter::ptr src) -> WavConverter::ptr
                 {
                    switch(bitsPerSample)
                      {
                       case WavBitsPerSample::_16:
                         return std::make_unique<ResamplePositiveIntegerFactor<s16, FACTOR>>(std::move(src));
                       case WavBitsPerSample::_32:
                         return std::make_unique<ResamplePositiveIntegerFactor<s32, FACTOR>>(std::move(src));
                       case WavBitsPerSample::_double:
                         return std::make_unique<ResamplePositiveIntegerFactor<double, FACTOR>>(std::move(src));
                       default:
                         std::cout << "WavConverter: unsupported bits per sample for resampling with positive integer factor: " << toUnderlying(bitsPerSample) << std::endl;
                         return nullptr;
                      }
                 }
          } // namespace
        
        // static
        auto WavConverter::buildConverter(const mods::utils::RBuffer<u8>& data, const Format& format, const StatCollector::sptr& statCollector, double peak) -> WavConverter::ptr
          {
             /* pipeline:
              sampleReader
              uncompress
              demux
              unpack or dummy
              upscale to fit container or dummy
              upscale or dummy
              resample or simple resample or dummy
              mix or dummy
              downscale or dummy
              mux
             */
             
             auto codec = format.getAudioFormat();
             auto bitsPerSample = format.getBitsPerSample();
             auto bitsPerContainer = format.getBitsPerContainer();
             auto nbChannels = format.getNumChannels();
             auto frequency = format.getSampleRate();
             auto channelMask = format.getChannelMask();
             
             u8 defaultValue = 0;
             switch(codec)
               {
                case WavAudioFormat::PCM:
                  if(std::find(PCMBitsPerSample.begin(), PCMBitsPerSample.end(), bitsPerSample) == PCMBitsPerSample.end())
                    {
                       std::cout << "Warning: invalid bits per sample for PCM: " << bitsPerSample << std::endl;
                    }
                  if(bitsPerSample == BITS_IN_BYTE)
                    {
                       defaultValue = default8BitsPCMValue;
                    }
                  break;
                  
                case WavAudioFormat::A_LAW:
                  defaultValue = ALawConverter::getZero();
                  break;
                  
                case WavAudioFormat::MU_LAW:
                  defaultValue = MuLawConverter::getZero();
                  break;
                  
                case WavAudioFormat::DVI_ADPCM:
                case WavAudioFormat::OKI_ADPCM:
                case WavAudioFormat::ADPCM:
                case WavAudioFormat::GSM:
                case WavAudioFormat::IEEE_FLOAT:
                case WavAudioFormat::TRUSPEECH:
                  break;
                  
                default:
                  std::cout << "WavConverter: unknown default value for " << toString(codec) << std::endl;
               }
             
             // read stage
             auto inputStream = std::make_unique<ReaderWavConverter>(data, defaultValue, statCollector);
             
             // uncompress stage
             auto uncompressedBitsPerContainer = static_cast<WavBitsPerContainer>(bitsPerContainer);
             std::vector<WavConverter::ptr> uncompressedChannels;
             bool floatingPointUncompressedStream = false;
             switch(codec)
               {
                case WavAudioFormat::PCM:
                  if(uncompressedBitsPerContainer == WavBitsPerContainer::_8)
                    {
                       uncompressedChannels.push_back(std::make_unique<UnsignedToSignedWavConverter<u8>>(std::move(inputStream)));
                    }
                  else
                    {
                       uncompressedChannels.push_back(std::move(inputStream));
                    }
                  break;
                  
                case WavAudioFormat::A_LAW:
                  uncompressedBitsPerContainer = WavBitsPerContainer::_16;
                  bitsPerSample = ALawConverter::getOutputBitsPerSample();
                  uncompressedChannels.push_back(std::make_unique<ALawConverter>(std::move(inputStream)));
                  break;
                  
                case WavAudioFormat::MU_LAW:
                  uncompressedBitsPerContainer = WavBitsPerContainer::_16;
                  bitsPerSample = ALawConverter::getOutputBitsPerSample();
                  uncompressedChannels.push_back(std::make_unique<MuLawConverter>(std::move(inputStream)));
                  break;
                  
                case WavAudioFormat::IEEE_FLOAT:
                  floatingPointUncompressedStream = true;
                  uncompressedChannels.push_back(std::move(inputStream));
                  break;
                  
                case WavAudioFormat::GSM:
                  if(nbChannels != 1)
                    {
                       std::cout << "Warning: GSM only supports 1 channel" << std::endl;
                    }
                  uncompressedBitsPerContainer = WavBitsPerContainer::_16;
                  bitsPerSample = GSMDecoderConverter::getOutputBitsPerSample();
                  uncompressedChannels.push_back(std::make_unique<GSMDecoderConverter>(std::move(inputStream)));
                  break;
                  
                case WavAudioFormat::TRUSPEECH:
                  if(nbChannels != 1)
                    {
                       std::cout << "Warning: Truespeech only supports 1 channel" << std::endl;
                    }
                  uncompressedBitsPerContainer = WavBitsPerContainer::_16;
                  bitsPerSample = TruspeechDecoderConverter::getOutputBitsPerSample();
                  uncompressedChannels.push_back(std::make_unique<TruspeechDecoderConverter>(std::move(inputStream)));
                  break;
                  
                case WavAudioFormat::DVI_ADPCM:
		  uncompressedBitsPerContainer = WavBitsPerContainer::_16;
		  bitsPerSample = DVIADPCMDecoderConverter::getOutputBitsPerSample();
                  uncompressedChannels.push_back(std::make_unique<DVIADPCMDecoderConverter>(std::move(inputStream), format));
		  break;
                  
                case WavAudioFormat::OKI_ADPCM:
                  uncompressedBitsPerContainer = WavBitsPerContainer::_16;
                  bitsPerSample = OKIADPCMDecoderConverter::getOutputBitsPerSample();
                  uncompressedChannels.push_back(std::make_unique<OKIADPCMDecoderConverter>(std::move(inputStream), format));
                  break;
                  
                case WavAudioFormat::ADPCM:
                  if(nbChannels > 2)
                    {
                       std::cout << "Warning: ADPCM only supports up to 2 channels" << std::endl;
                    }
                  uncompressedBitsPerContainer = WavBitsPerContainer::_16;
                  if(nbChannels == 2)
                    {
                       bitsPerSample = ADPCMDecoderConverter<2>::getOutputBitsPerSample();
                       uncompressedChannels.push_back(std::make_unique<ADPCMDecoderConverter<2>>(std::move(inputStream), format));
                    }
                  else
                    {
                       bitsPerSample = ADPCMDecoderConverter<1>::getOutputBitsPerSample();
                       uncompressedChannels.push_back(std::make_unique<ADPCMDecoderConverter<1>>(std::move(inputStream), format));
                    }
                  break;
                  
                default:
                  std::cout << "WavConverter: unknown codec for uncompressing stage: 0x" << std::hex << toUnderlying(codec) << std::dec << std::endl;
               }
             
             // demux stage
             auto demuxBitsPerContainer = uncompressedBitsPerContainer;
             std::vector<WavConverter::ptr> demuxedChannels;
             demuxedChannels.reserve(nbChannels);
             if(uncompressedChannels.size() == nbChannels)
               {
                  for(auto& channel : uncompressedChannels)
                    {
                       demuxedChannels.push_back(std::move(channel));
                    }
               }
             else
               {
                  if(uncompressedChannels.size() != 1)
                    {
                       std::cout << "Warning: we have several uncompressed channels but not the expected number" << std::endl;
                    }
                  
                  auto demuxConverter = std::make_unique<DemuxConverter>(std::move(uncompressedChannels.back()), nbChannels, toUnderlying(demuxBitsPerContainer));
                  auto* firstChannels = demuxConverter->getFirstChannels();
                  for(auto& channel : *firstChannels)
                    {
                       demuxedChannels.push_back(std::move(channel));
                    }
                  demuxedChannels.push_back(std::move(demuxConverter));
               }
             
             // unpack stage
             auto unpackedBitsPerContainer = demuxBitsPerContainer;
             std::vector<WavConverter::ptr> unpackedContainerChannels;
             unpackedContainerChannels.reserve(nbChannels);
             switch(uncompressedBitsPerContainer)
               {
                case WavBitsPerContainer::_8:
                case WavBitsPerContainer::_16:
                case WavBitsPerContainer::_32:
                case WavBitsPerContainer::_64:
                  for(size_t i = 0; i < nbChannels; ++i)
                    {
                       unpackedContainerChannels.push_back(std::move(demuxedChannels[i]));
                    }
                  break;
                  
                case WavBitsPerContainer::_24:
                  unpackedBitsPerContainer = WavBitsPerContainer::_32;
                  for(size_t i = 0; i < nbChannels; ++i)
                    {
                       unpackedContainerChannels.push_back(std::make_unique<UnpackToTypeConverter<s32>>(std::move(demuxedChannels[i]), 3));
                    }
                  break;
                  
                default:
                  std::cout << "WavConverter: unsupported " << toUnderlying(uncompressedBitsPerContainer) << " bits per container for unpacking container stage" << std::endl;
               }
             
             // scale bits per sample to container size stage
             auto unpackedBitsPerSample = static_cast<WavBitsPerSample>(unpackedBitsPerContainer);
             std::vector<WavConverter::ptr> unpackedSampleChannels;
             unpackedSampleChannels.reserve(nbChannels);
             if(toUnderlying(unpackedBitsPerContainer) == bitsPerSample)
               {
                  for(size_t i = 0; i < nbChannels; ++i)
                    {
                       unpackedSampleChannels.push_back(std::move(unpackedContainerChannels[i]));
                    }
               }
             else
               {
                  switch(unpackedBitsPerSample)
                    {
                     case WavBitsPerSample::_16:
                       for(size_t i = 0; i < nbChannels; ++i)
                         {
                            unpackedSampleChannels.push_back(std::make_unique<FillLSBConverter<s16>>(std::move(unpackedContainerChannels[i]), toUnderlying(unpackedBitsPerContainer) - bitsPerSample));
                         }
                       break;
                       
                     case WavBitsPerSample::_32:
                       for(size_t i = 0; i < nbChannels; ++i)
                         {
                            unpackedSampleChannels.push_back(std::make_unique<FillLSBConverter<s32>>(std::move(unpackedContainerChannels[i]), toUnderlying(unpackedBitsPerContainer) - bitsPerSample));
                         }
                       break;
                       
                     default:
                       std::cout << "WavConverter: unpacked bits per container is different than bits per sample (" << bitsPerSample << "/" << toUnderlying(unpackedBitsPerContainer) << ")" << std::endl;
                    }
               }
             
             // upscale stage
             auto upscaledBitsPerSample = unpackedBitsPerSample;
             std::vector<WavConverter::ptr> upscaledChannels;
             upscaledChannels.reserve(nbChannels);
             switch(unpackedBitsPerSample)
               {
                case WavBitsPerSample::_8:
                  if(isResamplableByPositiveIntegerFactor(frequency) && peak == 1.0)
                    {
                       upscaledBitsPerSample = WavBitsPerSample::_16;
                       for(size_t i=0; i < nbChannels; ++i)
                         {
                            upscaledChannels.push_back(std::make_unique<UpscaleWavConverter<s16, s8>>(std::move(unpackedSampleChannels[i])));
                         }
                    }
                  else
                    {
                       upscaledBitsPerSample = WavBitsPerSample::_double;
                       for(size_t i = 0; i < nbChannels; ++i)
                         {
                            upscaledChannels.push_back(std::make_unique<ToDoubleConverter<s8>>(std::move(unpackedSampleChannels[i])));
                         }
                    }
                  break;
                  
                case WavBitsPerSample::_16:
                  if(isResamplableByPositiveIntegerFactor(frequency) && peak == 1.0)
                    {
                       for(size_t i = 0; i < nbChannels; ++i)
                         {
                            upscaledChannels.push_back(std::move(unpackedSampleChannels[i]));
                         }
                    }
                  else
                    {
                       upscaledBitsPerSample = WavBitsPerSample::_double;
                       for(size_t i = 0; i < nbChannels; ++i)
                         {
                            upscaledChannels.push_back(std::make_unique<ToDoubleConverter<s16>>(std::move(unpackedSampleChannels[i])));
                         }
                    }
                  break;
                  
                case WavBitsPerSample::_32:
                  if(floatingPointUncompressedStream)
                    {
                       upscaledBitsPerSample = WavBitsPerSample::_double;
                       for(size_t i = 0; i < nbChannels; ++i)
                         {
                            upscaledChannels.push_back(std::make_unique<ToDoubleConverter<float>>(std::move(unpackedSampleChannels[i])));
                         }
                    }
                  else if(isResamplableByPositiveIntegerFactor(frequency) && peak == 1.0)
                    {
                       for(size_t i = 0; i < nbChannels; ++i)
                         {
                            upscaledChannels.push_back(std::move(unpackedSampleChannels[i]));
                         }
                    }
                  else
                    {
                       upscaledBitsPerSample = WavBitsPerSample::_double;
                       for(size_t i = 0; i < nbChannels; ++i)
                         {
                            upscaledChannels.push_back(std::make_unique<ToDoubleConverter<s32>>(std::move(unpackedSampleChannels[i])));
                         }
                    }
                  break;
                  
                case WavBitsPerSample::_64:
                  if(!floatingPointUncompressedStream)
                    {
                       std::cout << "WavConverter: unsuported codec in upscale stage for 64 bits sample" << std::endl;
                    }
                  else
                    {
                       upscaledBitsPerSample = WavBitsPerSample::_double;
                       for(size_t i = 0; i < nbChannels; ++i)
                         {
                            upscaledChannels.push_back(std::move(unpackedSampleChannels[i]));
                         }
                    }
                  break;
                  
                default:
                  std::cout << "WavConverter: unsupported bits per sample to upscale: " << toUnderlying(unpackedBitsPerSample) << std::endl;
               }
             
             // scale to peak stage
             if(peak != 1.0)
               {
                  std::vector<WavConverter::ptr> peakVec = std::move(upscaledChannels);
                  upscaledChannels.clear();
                  for(size_t i = 0; i < nbChannels; ++i)
                    {
                       upscaledChannels.emplace_back(std::make_unique<DivideConverter>(std::move(peakVec[i]), peak));
                    }
               }
             
             // resampling stage
             auto resampledBitsPerSample = upscaledBitsPerSample;
             std::vector<WavConverter::ptr> resampledChannels;
             resampledChannels.reserve(nbChannels);
             switch(static_cast<StandardFrequency>(frequency))
               {
                case StandardFrequency::_8000:
                  for(size_t i = 0; i < nbChannels; ++i)
                    {
                       using ParamType = StaticResampleParameters<StandardFrequency::_8000, StandardFrequency::_44100>;
                       ParamType params;
                       if(mods::utils::OpenCLManager::isEnabled())
                         {
                            using ResampleConverterImpl = OpenCLConverterTypes<ParamType>::ResampleConverterImpl;
                            resampledChannels.push_back(std::make_unique<ResampleConverterImpl>(std::move(upscaledChannels[i]), params));
                         }
                       else
                         {
                            resampledChannels.push_back(std::make_unique<SoftwareResampleConverter<ParamType>>(std::move(upscaledChannels[i]), params));
                         }
                    }
                  break;
		  
		case StandardFrequency::_10000:
		  for(size_t i = 0; i < nbChannels; ++i)
		    {
                       using ParamType = StaticResampleParameters<StandardFrequency::_10000, StandardFrequency::_44100>;
                       ParamType params;
                       if(mods::utils::OpenCLManager::isEnabled())
                         {
                            using ResampleConverterImpl = OpenCLConverterTypes<ParamType>::ResampleConverterImpl;
                            resampledChannels.push_back(std::make_unique<ResampleConverterImpl>(std::move(upscaledChannels[i]), params));
                         }
                       else
                         {
                            resampledChannels.push_back(std::make_unique<SoftwareResampleConverter<ParamType>>(std::move(upscaledChannels[i]), params));
                         }
		    }
		  break;
		  
		case StandardFrequency::_11025:
		  for(size_t i = 0; i < nbChannels; ++i)
		    {
		       resampledChannels.push_back(buildResamplePositiveIntegerFactor<4>(resampledBitsPerSample, std::move(upscaledChannels[i])));
		    }
		  break;
                  
                case StandardFrequency::_22000:
                  for(size_t i = 0; i < nbChannels; ++i)
                    {
                       using ParamType = StaticResampleParameters<StandardFrequency::_22000, StandardFrequency::_44100>;
                       ParamType params;
                       if(mods::utils::OpenCLManager::isEnabled())
                         {
                            using ResampleConverterImpl = OpenCLConverterTypes<ParamType>::ResampleConverterImpl;
                            resampledChannels.push_back(std::make_unique<ResampleConverterImpl>(std::move(upscaledChannels[i]), params));
                         }
                       else
                         {
                            resampledChannels.push_back(std::make_unique<SoftwareResampleConverter<ParamType>>(std::move(upscaledChannels[i]), params));
                         }
                    }
                  break;
                  
                case StandardFrequency::_22050:
                  for(size_t i = 0; i < nbChannels; ++i) 
                    {
                       resampledChannels.push_back(buildResamplePositiveIntegerFactor<2>(resampledBitsPerSample, std::move(upscaledChannels[i])));
                    }
                  break;
                  
                case StandardFrequency::_44100:
                  for(size_t i = 0; i < nbChannels; ++i)
                    {
                       resampledChannels.push_back(std::move(upscaledChannels[i]));
                    }
                  break;
                  
                case StandardFrequency::_48000:
                  for(size_t i = 0; i < nbChannels; ++i)
                    {
                       using ParamType = StaticResampleParameters<StandardFrequency::_48000, StandardFrequency::_44100>;
                       ParamType params;
                       if(mods::utils::OpenCLManager::isEnabled())
                         {
                            using ResampleConverterImpl = OpenCLConverterTypes<ParamType>::ResampleConverterImpl;
                            resampledChannels.push_back(std::make_unique<ResampleConverterImpl>(std::move(upscaledChannels[i]), params));
                         }
                       else
                         {
                            resampledChannels.push_back(std::make_unique<SoftwareResampleConverter<ParamType>>(std::move(upscaledChannels[i]), params));
                         }
                    }
                  break;
                  
                default:
                  std::cout << "WARNING: WavConverter: non standard frequency: " << frequency << ", using generic resampler" << std::endl;
		  for(size_t i = 0; i < nbChannels; ++i)
		    {
                       using ParamType = DynamicResampleParameters;
                       ParamType params(frequency, toUnderlying(StandardFrequency::_44100));
                       if(mods::utils::OpenCLManager::isEnabled())
                         {
                            using ResampleConverterImpl = OpenCLConverterTypes<ParamType>::ResampleConverterImpl;
                            resampledChannels.push_back(std::make_unique<ResampleConverterImpl>(std::move(upscaledChannels[i]), params));
                         }
                       else
                         {
                            resampledChannels.push_back(std::make_unique<SoftwareResampleConverter<ParamType>>(std::move(upscaledChannels[i]), params));
                         }
		    }
               }
             
             // mixing stage
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
                  floatChannels.reserve(nbChannels);
                  switch(resampledBitsPerSample)
                    {
                     case WavBitsPerSample::_16:
                       for(size_t i=0; i<nbChannels; ++i)
                         {
                            floatChannels.push_back(std::make_unique<ToDoubleConverter<s16>>(std::move(resampledChannels[i])));
                         }
                       break;
                     case WavBitsPerSample::_double:
                       for(size_t i=0; i<nbChannels; ++i)
                         {
                            floatChannels.push_back(std::move(resampledChannels[i]));
                         }
                       break;
                     default:
                       std::cout << "WavConverter: unsupported bits per sample in surround mixer:" << toUnderlying(resampledBitsPerSample) << std::endl;
                    }
                  resampledBitsPerSample = WavBitsPerSample::_double;
                  
                  auto mixer = std::make_unique<MultiChannelMixer>(std::move(floatChannels), channelMask);
                  mixedRight = mixer->getRightChannel();
                  mixedLeft = std::move(mixer);
               }
             
             // downscale stage
             ptr downScaledLeft;
             ptr downScaledRight;
             
             switch(resampledBitsPerSample)
               {
                case WavBitsPerSample::_16:
                  downScaledLeft = std::move(mixedLeft);
                  downScaledRight = std::move(mixedRight);
                  break;
                  
                case WavBitsPerSample::_32:
                  downScaledLeft = std::make_unique<DownscaleWavConverter<s16, s32>>(std::move(mixedLeft));
                  downScaledRight = std::make_unique<DownscaleWavConverter<s16, s32>>(std::move(mixedRight));
                  break;
                  
                case WavBitsPerSample::_double:
                  downScaledLeft = std::make_unique<FromDoubleConverter<s16>>(std::move(mixedLeft));
                  downScaledRight = std::make_unique<FromDoubleConverter<s16>>(std::move(mixedRight));
                  break;
                  
                default:
                  std::cout << "WavConverter: unsupported bits per sample to downscale: " << toUnderlying(resampledBitsPerSample) << std::endl;
               }
             
             // mux stage
             auto mux = std::make_unique<MultiplexerWavConverter>(std::move(downScaledLeft), std::move(downScaledRight));
             return mux;
          }
        
        // static
        auto WavConverter::isResamplableByPositiveIntegerFactor(int frequency) -> bool
          {
             auto f = static_cast<StandardFrequency>(frequency);
             return f == StandardFrequency::_44100 || f == StandardFrequency::_22050 || f == StandardFrequency::_11025;
          }
        
     } // namespace wav
} // namespace mods
