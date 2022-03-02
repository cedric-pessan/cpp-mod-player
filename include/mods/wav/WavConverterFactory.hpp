#ifndef MODS_WAV_WAVCONVERTERFACTORY_HPP
#define MODS_WAV_WAVCONVERTERFACTORY_HPP

#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/Format.hpp"
#include "mods/wav/StatCollector.hpp"

namespace mods
{
   namespace wav
     {
        class WavConverterFactory
          {
           public:             
             static auto buildConverter(const mods::utils::RBuffer<u8>& data, const Format& format, StatCollector* statCollector, double peak) -> mods::converters::Converter::ptr;
             
             WavConverterFactory() = delete;
             WavConverterFactory(const WavConverterFactory&) = delete;
             WavConverterFactory(WavConverterFactory&&) = delete;
             auto operator=(const WavConverterFactory&) -> WavConverterFactory& = delete;
             auto operator=(WavConverterFactory&&) -> WavConverterFactory& = delete;
              ~WavConverterFactory() = delete;
             
           private:
             static auto isResamplableByPositiveIntegerFactor(int frequency) -> bool;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_WAVCONVERTERFACTORY_HPP
