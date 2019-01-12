#ifndef MODS_WAV_WAVCONVERTER_HPP
#define MODS_WAV_WAVCONVERTER_HPP

#include "mods/utils/RBuffer.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/wav/StatCollector.hpp"

#include <memory>
#include <vector>

namespace mods
{
   namespace wav
     {
        class WavConverter
          {
           public:
             using ptr = std::unique_ptr<WavConverter>;
             
             static ptr buildConverter(const mods::utils::RBuffer<u8>& buffer, int bitsPerSample, int nbChannels, int frequency, StatCollector::sptr statCollector);
             
             WavConverter() = default;
             virtual ~WavConverter() = default;
             
             virtual bool isFinished() const = 0;
             virtual void read(mods::utils::RWBuffer<u8>* buf, int len) = 0;
             
             WavConverter(const WavConverter&) = delete;
             WavConverter(const WavConverter&&) = delete;
             WavConverter& operator=(const WavConverter&) = delete;
             WavConverter& operator=(const WavConverter&&) = delete;
             
           private:
             template<int FACTOR>
               static ptr buildResamplePositiveIntegerFactor(int bitsPerSample, ptr src);
             
             template<int BITSPERSAMPLE>
               static void buildDemuxStage(std::vector<WavConverter::ptr>* channels, int nbChannels, u8 defaultValue, const mods::utils::RBuffer<u8>& buffer, StatCollector::sptr statCollector);
             
             static bool isResamplableByPositiveIntegerFactor(int frequncy);
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_WAVCONVERTER_HPP
