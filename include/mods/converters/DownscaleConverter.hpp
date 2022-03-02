#ifndef MODS_CONVERTERS_DOWNSCALEWAVCONVERTER_HPP
#define MODS_CONVERTERS_DOWNSCALEWAVCONVERTER_HPP

#include "mods/converters/Converter.hpp"

#include <vector>

namespace mods
{
   namespace converters
     {
        template<typename TOut, typename TIn>
          class DownscaleConverter : public mods::converters::Converter
          {
           public:
             explicit DownscaleConverter(Converter::ptr src);
             
             DownscaleConverter() = delete;
             DownscaleConverter(const DownscaleConverter&) = delete;
             DownscaleConverter(DownscaleConverter&&) = delete;
             auto operator=(const DownscaleConverter&) -> DownscaleConverter& = delete;
             auto operator=(DownscaleConverter&&) -> DownscaleConverter& = delete;
             ~DownscaleConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             auto allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>;
             void ensureTempBufferSize(size_t len);
             
             auto constexpr shiftRight() -> u32
               {
                  return (sizeof(TIn) - sizeof(TOut))*BITS_IN_BYTE;
               }
             
             Converter::ptr _src;
             
             std::vector<u8> _tempVec;
             mods::utils::RWBuffer<u8> _temp;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_DOWNSCALEWAVCONVERTER_HPP
