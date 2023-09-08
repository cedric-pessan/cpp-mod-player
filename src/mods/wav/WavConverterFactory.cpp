
#include "mods/StandardFrequency.hpp"
#include "mods/converters/CastConverter.hpp"
#include "mods/converters/ChannelCopyConverter.hpp"
#include "mods/converters/Converter.hpp"
#include "mods/converters/DivideConverter.hpp"
#include "mods/converters/DownscaleConverter.hpp"
#include "mods/converters/FillLSBConverter.hpp"
#include "mods/converters/FromDoubleConverter.hpp"
#include "mods/converters/MultiplexerConverter.hpp"
#include "mods/converters/OpenCLConverterTypes.hpp"
#include "mods/converters/ResampleParameters.hpp"
#include "mods/converters/ResamplePositiveIntegerFactor.hpp"
#include "mods/converters/SoftwareResampleConverter.hpp"
#include "mods/converters/ToDoubleConverter.hpp"
#include "mods/converters/UnsignedToSignedConverter.hpp"
#include "mods/converters/UpscaleConverter.hpp"
#include "mods/utils/OpenCLManager.hpp"
#include "mods/wav/ADPCMDecoderConverter.hpp"
#include "mods/wav/ALawConverter.hpp"
#include "mods/wav/DVIADPCMDecoderConverter.hpp"
#include "mods/wav/DemuxConverter.hpp"
#include "mods/wav/GSMDecoderConverter.hpp"
#include "mods/wav/MuLawConverter.hpp"
#include "mods/wav/MultiChannelMixer.hpp"
#include "mods/wav/OKIADPCMDecoderConverter.hpp"
#include "mods/wav/ReaderWavConverter.hpp"
#include "mods/wav/TruspeechDecoderConverter.hpp"
#include "mods/wav/UnpackToTypeConverter.hpp"
#include "mods/wav/WavConverterFactory.hpp"

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
          } // namespace
        
        // static
        auto WavConverterFactory::buildConverter(const mods::utils::RBuffer<u8>& data, const Format& format, StatCollector* statCollector, double peak) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::CastConverter;
             using mods::converters::Converter;
             using mods::converters::UnsignedToSignedConverter;
             
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
             switch(codec)
               {
                case WavAudioFormat::PCM:
                  if(uncompressedBitsPerContainer == WavBitsPerContainer::_8)
                    {
                       std::vector<Converter<s8>::ptr> uncompressedChannels;
                       uncompressedChannels.push_back(std::make_unique<UnsignedToSignedConverter<s8, u8>>(std::move(inputStream)));
                       return buildDemuxStage<s8>(std::move(uncompressedChannels), nbChannels, uncompressedBitsPerContainer, bitsPerSample, frequency, peak, channelMask, false);
                    }
                  else
                    {
                       std::vector<Converter<u8>::ptr> uncompressedChannels;
                       uncompressedChannels.push_back(std::move(inputStream));
                       return buildDemuxStage<u8>(std::move(uncompressedChannels), nbChannels, uncompressedBitsPerContainer, bitsPerSample, frequency, peak, channelMask, false);
                    }
                  
                case WavAudioFormat::A_LAW:
                    {
                       std::vector<Converter<s16>::ptr> uncompressedChannels;
                       uncompressedChannels.push_back(std::make_unique<ALawConverter>(std::move(inputStream)));
                       return buildDemuxStage<s16>(std::move(uncompressedChannels), nbChannels, WavBitsPerContainer::_16, ALawConverter::getOutputBitsPerSample(), frequency, peak, channelMask, false);
                    }
                  
                case WavAudioFormat::MU_LAW:
                    {
                       std::vector<Converter<s16>::ptr> uncompressedChannels;
                       uncompressedChannels.push_back(std::make_unique<MuLawConverter>(std::move(inputStream)));
                       return buildDemuxStage<s16>(std::move(uncompressedChannels), nbChannels, WavBitsPerContainer::_16, MuLawConverter::getOutputBitsPerSample(), frequency, peak, channelMask, false);
                    }
                  
                case WavAudioFormat::IEEE_FLOAT:
                    {
                       std::vector<Converter<u8>::ptr> uncompressedChannels;
                       uncompressedChannels.push_back(std::move(inputStream));
                       return buildDemuxStage<u8>(std::move(uncompressedChannels), nbChannels, uncompressedBitsPerContainer, bitsPerSample, frequency, peak, channelMask, true);
                    }
                  
                case WavAudioFormat::GSM:
                    {
                       if(nbChannels != 1)
                         {
                            std::cout << "Warning: GSM only supports 1 channel" << std::endl;
                         }
                       std::vector<Converter<s16>::ptr> uncompressedChannels;
                       uncompressedChannels.push_back(std::make_unique<GSMDecoderConverter>(std::move(inputStream)));
                       return buildScaleToContainerSizeStage<s16>(std::move(uncompressedChannels), WavBitsPerContainer::_16, GSMDecoderConverter::getOutputBitsPerSample(), frequency, peak, channelMask);
                    }
                  
                case WavAudioFormat::TRUSPEECH:
                    {
                       if(nbChannels != 1)
                         {
                            std::cout << "Warning: Truespeech only supports 1 channel" << std::endl;
                         }
                       std::vector<Converter<s16>::ptr> uncompressedChannels;
                       uncompressedChannels.push_back(std::make_unique<TruspeechDecoderConverter>(std::move(inputStream)));
                       return buildScaleToContainerSizeStage<s16>(std::move(uncompressedChannels), WavBitsPerContainer::_16, TruspeechDecoderConverter::getOutputBitsPerSample(), frequency, peak, channelMask);
                    }
                  
                case WavAudioFormat::DVI_ADPCM:
                    {
                       std::vector<Converter<s16>::ptr> uncompressedChannels;
                       uncompressedChannels.push_back(std::make_unique<DVIADPCMDecoderConverter>(std::move(inputStream), format));
                       return buildDemuxStage<s16>(std::move(uncompressedChannels), nbChannels, WavBitsPerContainer::_16, DVIADPCMDecoderConverter::getOutputBitsPerSample(), frequency, peak, channelMask, false);
                    }
                  
                case WavAudioFormat::OKI_ADPCM:
                    {
                       std::vector<Converter<s16>::ptr> uncompressedChannels;
                       uncompressedChannels.push_back(std::make_unique<OKIADPCMDecoderConverter>(std::move(inputStream), format));
                       return buildDemuxStage<s16>(std::move(uncompressedChannels), nbChannels, WavBitsPerContainer::_16, OKIADPCMDecoderConverter::getOutputBitsPerSample(), frequency, peak, channelMask, false);
                    }
                  
                case WavAudioFormat::ADPCM:
                    {
                       if(nbChannels > 2)
                         {
                            std::cout << "Warning: ADPCM only supports up to 2 channels" << std::endl;
                         }
                       
                       std::vector<Converter<s16>::ptr> uncompressedChannels;
                       int bitsPerSample;
                       
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
                       return buildDemuxStage<s16>(std::move(uncompressedChannels), nbChannels, WavBitsPerContainer::_16, bitsPerSample, frequency, peak, channelMask, false);
                    }
                  
                default:
                  std::cout << "WavConverter: unknown codec for uncompressing stage: 0x" << std::hex << toUnderlying(codec) << std::dec << std::endl;
                  return nullptr;
               }
          }
        
        // static
        template<typename T>
          auto WavConverterFactory::buildDemuxStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels,
                                                    size_t nbChannels,
                                                    WavBitsPerContainer bitsPerContainer,
                                                    u16 bitsPerSample,
                                                    u32 frequency,
                                                    double peak,
                                                    u32 channelMask,
                                                    bool floatingPointStream) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::Converter;
             
             if(channels.size() == nbChannels)
               {
                  return buildUnpackStage<T>(std::move(channels), bitsPerContainer, bitsPerSample, frequency, peak, channelMask, floatingPointStream);
               }
             else
               {
                  if(channels.size() != 1)
                    {
                       std::cout << "Warning: we have several uncompressed channels but not the expected number" << std::endl;
                    }
                  
                  auto demuxConverter = std::make_unique<DemuxConverter<T>>(std::move(channels.back()), nbChannels, toUnderlying(bitsPerContainer));
                  auto* firstChannels = demuxConverter->getFirstChannels();
                  
                  std::vector<typename Converter<T>::ptr> demuxedChannels;
                  demuxedChannels.reserve(nbChannels);
                  for(auto& channel : *firstChannels)
                    {
                       demuxedChannels.push_back(std::move(channel));
                    }
                  demuxedChannels.push_back(std::move(demuxConverter));
                  return buildUnpackStage<T>(std::move(demuxedChannels), bitsPerContainer, bitsPerSample, frequency, peak, channelMask, floatingPointStream);
               }
          }
        
        // static
        template<typename T>
          auto WavConverterFactory::buildUnpackStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels,
                                                     WavBitsPerContainer bitsPerContainer,
                                                     u16 bitsPerSample,
                                                     u32 frequency,
                                                     double peak,
                                                     u32 channelMask,
                                                     bool floatingPointStream) -> typename std::enable_if<std::is_signed<T>::value, mods::converters::Converter<s16>::ptr>::type
          {
             return buildScaleToContainerSizeStage<T>(std::move(channels), bitsPerContainer, bitsPerSample, frequency, peak, channelMask);
          }
        
        // static
        template<typename T>
          auto WavConverterFactory::buildUnpackStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels,
                                                     WavBitsPerContainer bitsPerContainer,
                                                     u16 bitsPerSample,
                                                     u32 frequency,
                                                     double peak,
                                                     u32 channelMask,
                                                     bool floatingPointStream) -> typename std::enable_if<std::is_unsigned<T>::value, mods::converters::Converter<s16>::ptr>::type
          {
             using mods::converters::CastConverter;
             using mods::converters::Converter;
             
             if(floatingPointStream)
               {
                  if(toUnderlying(bitsPerContainer) != bitsPerSample)
                    {
                       std::cout << "WavConverter: bits per sample should be equal to bits per container for floating point streams" << std::endl;
                    }
                  
                  switch(bitsPerContainer)
                    {
                     case WavBitsPerContainer::_32:
                         {
                            std::vector<Converter<float>::ptr> castedChannels;
                            castedChannels.reserve(channels.size());
                            for(auto& channel : channels)
                              {
                                 castedChannels.push_back(std::make_unique<CastConverter<float, T>>(std::move(channel)));
                              }
                            return buildUpscaleStage<float>(std::move(castedChannels), frequency, peak, channelMask);
                         }
                       
                     case WavBitsPerContainer::_64:
                         {
                            std::vector<Converter<double>::ptr> castedChannels;
                            castedChannels.reserve(channels.size());
                            for(auto& channel : channels)
                              {
                                 castedChannels.push_back(std::make_unique<CastConverter<double, T>>(std::move(channel)));
                              }
                            return buildUpscaleStage<double>(std::move(castedChannels), frequency, peak, channelMask);
                         }
                       
                     default:
                       std::cout << "WavConverter: unsupported " << toUnderlying(bitsPerContainer) << " bits per container for unpacking floating point container stage" << std::endl;
                       return nullptr;
                    }
               }
             else
               {
                  switch(bitsPerContainer)
                    {
                     case WavBitsPerContainer::_16:
                         {
                            std::vector<Converter<s16>::ptr> castedChannels;
                            castedChannels.reserve(channels.size());
                            for(auto& channel : channels)
                              {
                                 castedChannels.push_back(std::make_unique<CastConverter<s16, T>>(std::move(channel)));
                              }
                            return buildScaleToContainerSizeStage<s16>(std::move(castedChannels), bitsPerContainer, bitsPerSample, frequency, peak, channelMask);
                         }
                       
                     case WavBitsPerContainer::_24:
                         {
                            std::vector<Converter<s32>::ptr> unpackedChannels;
                            unpackedChannels.reserve(channels.size());
                            for(auto& channel : channels)
                              {
                                 unpackedChannels.push_back(std::make_unique<UnpackToTypeConverter<s32>>(std::move(channel), 3));
                              }
                            return buildScaleToContainerSizeStage<s32>(std::move(unpackedChannels), WavBitsPerContainer::_32, bitsPerSample, frequency, peak, channelMask);
                         }
                       
                     case WavBitsPerContainer::_32:
                         {
                            std::vector<Converter<s32>::ptr> castedChannels;
                            castedChannels.reserve(channels.size());
                            for(auto& channel : channels)
                              {
                                 castedChannels.push_back(std::make_unique<CastConverter<s32, T>>(std::move(channel)));
                              }
                            return buildScaleToContainerSizeStage<s32>(std::move(castedChannels), bitsPerContainer, bitsPerSample, frequency, peak, channelMask);
                         }
                       
                     default:
                       std::cout << "WavConverter: unsupported " << toUnderlying(bitsPerContainer) << " bits per container for unpacking container stage (unsigned type)" << std::endl;
                       return nullptr;
                    }
               }
          }
        
        // static
        template<typename T>
          auto WavConverterFactory::buildScaleToContainerSizeStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels,
                                                                   WavBitsPerContainer bitsPerContainer,
                                                                   u16 bitsPerSample,
                                                                   u32 frequency,
                                                                   double peak,
                                                                   u32 channelMask) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::Converter;
             using mods::converters::FillLSBConverter;
             
             if(toUnderlying(bitsPerContainer) == bitsPerSample)
               {
                  return buildUpscaleStage<T>(std::move(channels), frequency, peak, channelMask);
               }
             else
               {
                  std::vector<typename Converter<T>::ptr> scaledChannels;
                  scaledChannels.reserve(channels.size());
                  for(auto& channel : channels)
                    {
                       scaledChannels.push_back(std::make_unique<FillLSBConverter<T>>(std::move(channel), toUnderlying(bitsPerContainer) - bitsPerSample));
                    }
                  return buildUpscaleStage<T>(std::move(scaledChannels), frequency, peak, channelMask);
               }
          }
        
        // static
        template<>
          auto WavConverterFactory::buildUpscaleStage<s8>(std::vector<typename mods::converters::Converter<s8>::ptr>&& channels,
                                                          u32 frequency,
                                                          double peak,
                                                          u32 channelMask) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::Converter;
             using mods::converters::ToDoubleConverter;
             using mods::converters::UpscaleConverter;
             
             if(isResamplableByPositiveIntegerFactor(frequency) && peak == 1.0)
               {
                  std::vector<Converter<s16>::ptr> upscaledChannels;
                  upscaledChannels.reserve(channels.size());
                  for(auto& channel : channels)
                    {
                       upscaledChannels.push_back(std::make_unique<UpscaleConverter<s16, s8>>(std::move(channel)));
                    }
                  return buildPositiveIntegerResamplingStage<s16>(std::move(upscaledChannels), frequency, channelMask);
               }
             else
               {
                  std::vector<Converter<double>::ptr> upscaledChannels;
                  upscaledChannels.reserve(channels.size());
                  for(auto& channel: channels)
                    {
                       upscaledChannels.push_back(std::make_unique<ToDoubleConverter<s8>>(std::move(channel)));
                    }
                  return buildPeakStage(std::move(upscaledChannels), frequency, peak, channelMask);
               }
          }
        
        // static
        template<>
          auto WavConverterFactory::buildUpscaleStage<s16>(std::vector<typename mods::converters::Converter<s16>::ptr>&& channels,
                                                           u32 frequency,
                                                           double peak,
                                                           u32 channelMask) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::Converter;
             using mods::converters::ToDoubleConverter;
             
             if(isResamplableByPositiveIntegerFactor(frequency) && peak == 1.0)
               {
                  return buildPositiveIntegerResamplingStage<s16>(std::move(channels), frequency, channelMask);
               }
             else
               {
                  std::vector<Converter<double>::ptr> upscaledChannels;
                  upscaledChannels.reserve(channels.size());
                  for(auto& channel: channels)
                    {
                       upscaledChannels.push_back(std::make_unique<ToDoubleConverter<s16>>(std::move(channel)));
                    }
                  return buildPeakStage(std::move(upscaledChannels), frequency, peak, channelMask);
               }
          }
        
        // static
        template<>
          auto WavConverterFactory::buildUpscaleStage<s32>(std::vector<typename mods::converters::Converter<s32>::ptr>&& channels,
                                                           u32 frequency,
                                                           double peak,
                                                           u32 channelMask) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::Converter;
             using mods::converters::ToDoubleConverter;
             
             if(isResamplableByPositiveIntegerFactor(frequency) && peak == 1.0)
               {
                  return buildPositiveIntegerResamplingStage<s32>(std::move(channels), frequency, channelMask);
               }
             else
               {
                  std::vector<Converter<double>::ptr> upscaledChannels;
                  upscaledChannels.reserve(channels.size());
                  for(auto& channel : channels)
                    {
                       upscaledChannels.push_back(std::make_unique<ToDoubleConverter<s32>>(std::move(channel)));
                    }
                  return buildPeakStage(std::move(upscaledChannels), frequency, peak, channelMask);
               }
          }
        
        // static
        template<>
          auto WavConverterFactory::buildUpscaleStage<float>(std::vector<typename mods::converters::Converter<float>::ptr>&& channels,
                                                             u32 frequency,
                                                             double peak,
                                                             u32 channelMask) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::Converter;
             using mods::converters::ToDoubleConverter;
             
             std::vector<Converter<double>::ptr> upscaledChannels;
             upscaledChannels.reserve(channels.size());
             for(auto& channel : channels)
               {
                  upscaledChannels.push_back(std::make_unique<ToDoubleConverter<float>>(std::move(channel)));
               }
             return buildPeakStage(std::move(upscaledChannels), frequency, peak, channelMask);
          }
        
        // static
        template<>
          auto WavConverterFactory::buildUpscaleStage<double>(std::vector<typename mods::converters::Converter<double>::ptr>&& channels,
                                                              u32 frequency,
                                                              double peak,
                                                              u32 channelMask) -> mods::converters::Converter<s16>::ptr
          {
             return buildPeakStage(std::move(channels), frequency, peak, channelMask);
          }
        
        // static
        auto WavConverterFactory::buildPeakStage(std::vector<mods::converters::Converter<double>::ptr>&& channels,
                                                 u32 frequency,
                                                 double peak,
                                                 u32 channelMask) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::Converter;
             using mods::converters::DivideConverter;
             
             if(peak != 1.0)
               {
                  std::vector<Converter<double>::ptr> peakCorrectedChannels;
                  peakCorrectedChannels.reserve(channels.size());
                  for(auto& channel : channels)
                    {
                       peakCorrectedChannels.emplace_back(std::make_unique<DivideConverter>(std::move(channel), peak));
                    }
                  return buildResamplingStage(std::move(peakCorrectedChannels), frequency, channelMask);
               }
             return buildResamplingStage(std::move(channels), frequency, channelMask);
          }
        
        // static
        auto WavConverterFactory::buildResamplingStage(std::vector<typename mods::converters::Converter<double>::ptr>&& channels,
                                                       u32 frequency,
                                                       u32 channelMask) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::Converter;
             using mods::converters::DynamicResampleParameters;
             using mods::converters::OpenCLConverterTypes;
             using mods::converters::SoftwareResampleConverter;
             using mods::converters::StaticResampleParameters;
             
             std::vector<typename Converter<double>::ptr> resampledChannels;
             resampledChannels.reserve(channels.size());
             switch(static_cast<StandardFrequency>(frequency))
               {
                case StandardFrequency::_8000:
                  for(auto& channel : channels)
                    {
                       using ParamType = StaticResampleParameters<StandardFrequency::_8000, StandardFrequency::_44100>;
                       ParamType params;
                       if(mods::utils::OpenCLManager::isEnabled())
                         {
                            using ResampleConverterImpl = OpenCLConverterTypes<ParamType, double>::ResampleConverterImpl;
                            resampledChannels.push_back(std::make_unique<ResampleConverterImpl>(std::move(channel), params));
                         }
                       else
                         {
                            resampledChannels.push_back(std::make_unique<SoftwareResampleConverter<ParamType, double>>(std::move(channel), params));
                         }
                    }
                  return buildMixingStage<double>(std::move(resampledChannels), channelMask);
		  
		case StandardFrequency::_10000:
                  for(auto& channel : channels)
		    {
                       using ParamType = StaticResampleParameters<StandardFrequency::_10000, StandardFrequency::_44100>;
                       ParamType params;
                       if(mods::utils::OpenCLManager::isEnabled())
                         {
                            using ResampleConverterImpl = OpenCLConverterTypes<ParamType, double>::ResampleConverterImpl;
                            resampledChannels.push_back(std::make_unique<ResampleConverterImpl>(std::move(channel), params));
                         }
                       else
                         {
                            resampledChannels.push_back(std::make_unique<SoftwareResampleConverter<ParamType, double>>(std::move(channel), params));
                         }
		    }
                  return buildMixingStage<double>(std::move(resampledChannels), channelMask);
                  
                case StandardFrequency::_11025:
                  return buildPositiveIntegerResamplingStage<double>(std::move(channels), frequency, channelMask);
                  
                case StandardFrequency::_22000:
                  for(auto& channel : channels)
                    {
                       using ParamType = StaticResampleParameters<StandardFrequency::_22000, StandardFrequency::_44100>;
                       ParamType params;
                       if(mods::utils::OpenCLManager::isEnabled())
                         {
                            using ResampleConverterImpl = typename OpenCLConverterTypes<ParamType, double>::ResampleConverterImpl;
                            resampledChannels.push_back(std::make_unique<ResampleConverterImpl>(std::move(channel), params));
                         }
                       else
                         {
                            resampledChannels.push_back(std::make_unique<SoftwareResampleConverter<ParamType, double>>(std::move(channel), params));
                         }
                    }
                  return buildMixingStage<double>(std::move(resampledChannels), channelMask);
                  
                case StandardFrequency::_22050:
                  return buildPositiveIntegerResamplingStage<double>(std::move(channels), frequency, channelMask);
                  
                case StandardFrequency::_48000:
                  for(auto& channel : channels)
                    {
                       using ParamType = StaticResampleParameters<StandardFrequency::_48000, StandardFrequency::_44100>;
                       ParamType params;
                       if(mods::utils::OpenCLManager::isEnabled())
                         {
                            using ResampleConverterImpl = OpenCLConverterTypes<ParamType, double>::ResampleConverterImpl;
                            resampledChannels.push_back(std::make_unique<ResampleConverterImpl>(std::move(channel), params));
                         }
                       else
                         {
                            resampledChannels.push_back(std::make_unique<SoftwareResampleConverter<ParamType, double>>(std::move(channel), params));
                         }
                    }
                  return buildMixingStage<double>(std::move(resampledChannels), channelMask);
                  
                case StandardFrequency::_44100:
                  return buildPositiveIntegerResamplingStage<double>(std::move(channels), frequency, channelMask);
                  
                default:
                  std::cout << "WARNING: WavConverter: non standard frequency: " << frequency << ", using generic resampler" << std::endl;
                  for(auto& channel : channels)
		    {
                       using ParamType = DynamicResampleParameters;
                       ParamType params(frequency, toUnderlying(StandardFrequency::_44100));
                       if(mods::utils::OpenCLManager::isEnabled())
                         {
                            using ResampleConverterImpl = OpenCLConverterTypes<ParamType, double>::ResampleConverterImpl;
                            resampledChannels.push_back(std::make_unique<ResampleConverterImpl>(std::move(channel), params));
                         }
                       else
                         {
                            resampledChannels.push_back(std::make_unique<SoftwareResampleConverter<ParamType, double>>(std::move(channel), params));
                         }
		    }
                  return buildMixingStage<double>(std::move(resampledChannels), channelMask);
               }
          }
        
        // static
        template<typename T>
          auto WavConverterFactory::buildPositiveIntegerResamplingStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels,
                                                                        u32 frequency,
                                                                        u32 channelMask) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::Converter;
             using mods::converters::ResamplePositiveIntegerFactor;
             
             std::vector<typename Converter<T>::ptr> resampledChannels;
             resampledChannels.reserve(channels.size());
             switch(static_cast<StandardFrequency>(frequency))
               {
		case StandardFrequency::_11025:
                  for(auto& channel : channels)
		    {
                       resampledChannels.push_back(std::make_unique<ResamplePositiveIntegerFactor<T, 4>>(std::move(channel)));
		    }
                  return buildMixingStage<T>(std::move(resampledChannels), channelMask);
                  
                case StandardFrequency::_22050:
                  for(auto& channel : channels)
                    {
                       resampledChannels.push_back(std::make_unique<ResamplePositiveIntegerFactor<T, 2>>(std::move(channel)));
                    }
                  return buildMixingStage<T>(std::move(resampledChannels), channelMask);
                  
                case StandardFrequency::_44100:
                  return buildMixingStage<T>(std::move(channels), channelMask);
                  
                default:
                  std::cout << "WARNING: WavConverter: only integer resampling supported for types that are not 'double' and found frequency: " << frequency << std::endl;
                  return nullptr;
               }
          }
        
        // static
        template<typename T>
          auto WavConverterFactory::buildMixingStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels,
                                                     u32 channelMask) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::ChannelCopyConverter;
             
             auto nbChannels = channels.size();
             
             typename converters::Converter<T>::ptr left;
             typename converters::Converter<T>::ptr right;
             
             if(channelMask == 0)
               {
                  switch(nbChannels)
                    {
                     case 1:
                         {
                            auto duplicator = std::make_unique<ChannelCopyConverter<T>>(std::move(channels[0]));
                            right = duplicator->getCopy();
                            left = std::move(duplicator);
                         }
                       return buildDownscaleStage<T>(std::move(left), std::move(right));
                       
                     case 2:
                       return buildDownscaleStage<T>(std::move(channels[0]), std::move(channels[1]));
                       break;
                       
                     default:
                       std::cout << "WavConverter: unsupported number of channels for mixing stage: " << nbChannels << std::endl;
                       return nullptr;
                    }
               }
             else
               {
                  auto floatChannels = buildConvertToDoubleStage<T>(std::move(channels));
                  
                  auto mixer = std::make_unique<MultiChannelMixer>(std::move(floatChannels), channelMask);
                  auto mixedRight = mixer->getRightChannel();
                  auto& mixedLeft = mixer;
                  return buildDownscaleStage<double>(std::move(mixedLeft), std::move(mixedRight));
               }
          }
        
        // static
        template<>
          auto WavConverterFactory::buildConvertToDoubleStage<double>(std::vector<typename mods::converters::Converter<double>::ptr>&& channels) -> std::vector<mods::converters::Converter<double>::ptr>
          {
             return std::move(channels);
          }
        
        // static
        template<typename T>
          auto WavConverterFactory::buildConvertToDoubleStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels) -> std::vector<mods::converters::Converter<double>::ptr>
          {
             using mods::converters::Converter;
             using mods::converters::ToDoubleConverter;
             
             std::vector<Converter<double>::ptr> floatChannels;
             floatChannels.reserve(channels.size());
             for(auto& channel : channels)
               {
                  floatChannels.push_back(std::make_unique<ToDoubleConverter<T>>(std::move(channel)));
               }
             
             return floatChannels;
          }
        
        // static
        template<>
          auto WavConverterFactory::buildDownscaleStage<s16>(typename mods::converters::Converter<s16>::ptr left,
                                                             typename mods::converters::Converter<s16>::ptr right) -> mods::converters::Converter<s16>::ptr
          {
             return buildMuxStage(std::move(left), std::move(right));
          }
        
        // static
        template<>
          auto WavConverterFactory::buildDownscaleStage<double>(typename mods::converters::Converter<double>::ptr left,
                                                                typename mods::converters::Converter<double>::ptr right) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::FromDoubleConverter;
             
             return buildMuxStage(std::make_unique<FromDoubleConverter<s16>>(std::move(left)),
                                  std::make_unique<FromDoubleConverter<s16>>(std::move(right)));
          }
        
        // static
        template<>
          auto WavConverterFactory::buildDownscaleStage<s32>(typename mods::converters::Converter<s32>::ptr left,
                                                             typename mods::converters::Converter<s32>::ptr right) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::DownscaleConverter;
             
             return buildMuxStage(std::make_unique<DownscaleConverter<s16, s32>>(std::move(left)),
                                  std::make_unique<DownscaleConverter<s16, s32>>(std::move(right)));
          }
        
        // static
        auto WavConverterFactory::buildMuxStage(mods::converters::Converter<s16>::ptr left,
                                                mods::converters::Converter<s16>::ptr right) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::MultiplexerConverter;
             
             return std::make_unique<MultiplexerConverter<s16>>(std::move(left), std::move(right));
          }
        
        // static
        auto WavConverterFactory::isResamplableByPositiveIntegerFactor(int frequency) -> bool
          {
             auto f = static_cast<StandardFrequency>(frequency);
             return f == StandardFrequency::_44100 || f == StandardFrequency::_22050 || f == StandardFrequency::_11025;
          }
        
     } // namespace wav
} // namespace mods
