#ifndef MODS_WAV_WAVCONVERTERFACTORY_HPP
#define MODS_WAV_WAVCONVERTERFACTORY_HPP

#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/Format.hpp"
#include "mods/wav/StatCollector.hpp"

#include <type_traits>

namespace mods
{
   namespace wav
     {
        class WavConverterFactory
          {
           public:             
             static auto buildConverter(const mods::utils::RBuffer<u8>& data, const Format& format, StatCollector* statCollector, double peak) -> mods::converters::Converter<s16>::ptr;
             
             WavConverterFactory() = delete;
             WavConverterFactory(const WavConverterFactory&) = delete;
             WavConverterFactory(WavConverterFactory&&) = delete;
             auto operator=(const WavConverterFactory&) -> WavConverterFactory& = delete;
             auto operator=(WavConverterFactory&&) -> WavConverterFactory& = delete;
              ~WavConverterFactory() = delete;
             
           private:
             static auto isResamplableByPositiveIntegerFactor(int frequency) -> bool;
             
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
                    _64 = 64
               };
             
             template<typename T>
               static auto buildDemuxStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels,
                                           size_t nbChannels,
                                           WavBitsPerContainer bitsPerContainer,
                                           u16 bitsPerSample,
                                           u32 frequency,
                                           double peak,
                                           u32 channelMask,
                                           bool floatingPointStream) -> mods::converters::Converter<s16>::ptr;
             
             template<typename T>
               static auto buildUnpackStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels,
                                            WavBitsPerContainer bitsPerContainer,
                                            u16 bitsPerSample,
                                            u32 frequency,
                                            double peak,
                                            u32 channelMask,
                                            bool floatingPointStream) -> typename std::enable_if<std::is_signed<T>::value, mods::converters::Converter<s16>::ptr>::type;
             
             template<typename T>
               static auto buildUnpackStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels,
                                            WavBitsPerContainer bitsPerContainer,
                                            u16 bitsPerSample,
                                            u32 frequency,
                                            double peak,
                                            u32 channelMask,
                                            bool floatingPointStream) -> typename std::enable_if<std::is_unsigned<T>::value, mods::converters::Converter<s16>::ptr>::type;
             
             template<typename T>
               static auto buildScaleToContainerSizeStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels,
                                                          WavBitsPerContainer bitsPerContainer,
                                                          u16 bitsPerSample,
                                                          u32 frequency,
                                                          double peak,
                                                          u32 channelMask) -> mods::converters::Converter<s16>::ptr;
             
             template<typename T>
               static auto buildUpscaleStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels,
                                             u32 frequency,
                                             double peak,
                                             u32 channelMask) -> mods::converters::Converter<s16>::ptr;
             
             static auto buildPeakStage(std::vector<mods::converters::Converter<double>::ptr>&& channels,
                                        u32 frequency,
                                        double peak,
                                        u32 channelMask) -> mods::converters::Converter<s16>::ptr;
             
             static auto buildResamplingStage(std::vector<typename mods::converters::Converter<double>::ptr>&& channels,
                                              u32 frequency,
                                              u32 channelMask) -> mods::converters::Converter<s16>::ptr;
             
             template<typename T>
               static auto buildPositiveIntegerResamplingStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels,
                                                               u32 frequency,
                                                               u32 channelMask) -> mods::converters::Converter<s16>::ptr;
             
             template<typename T>
               static auto buildMixingStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels,
                                            u32 channelMask) -> mods::converters::Converter<s16>::ptr;
             
             template<typename T>
               static auto buildDownscaleStage(typename mods::converters::Converter<T>::ptr left,
                                               typename mods::converters::Converter<T>::ptr right) -> mods::converters::Converter<s16>::ptr;
             
             static auto buildMuxStage(mods::converters::Converter<s16>::ptr left,
                                       mods::converters::Converter<s16>::ptr right) -> mods::converters::Converter<s16>::ptr;
             
             template<typename T>
               static auto buildConvertToDoubleStage(std::vector<typename mods::converters::Converter<T>::ptr>&& channels) -> std::vector<mods::converters::Converter<double>::ptr>;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_WAVCONVERTERFACTORY_HPP
