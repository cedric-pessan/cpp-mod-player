#ifndef MODS_CONVERTERS_DOWNSCALECONVERTER_HPP
#define MODS_CONVERTERS_DOWNSCALECONVERTER_HPP

#include "mods/converters/Converter.hpp"

#include <vector>

namespace mods
{
   namespace converters
     {
        template<typename TOut, typename TIn>
          class DownscaleConverter : public mods::converters::Converter<TOut>
          {
           public:
             explicit DownscaleConverter(typename Converter<TIn>::ptr src);
             
             DownscaleConverter() = delete;
             DownscaleConverter(const DownscaleConverter&) = delete;
             DownscaleConverter(DownscaleConverter&&) = delete;
             auto operator=(const DownscaleConverter&) -> DownscaleConverter& = delete;
             auto operator=(DownscaleConverter&&) -> DownscaleConverter& = delete;
             ~DownscaleConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<TOut>* buf) override;
             
           private:
             auto allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>;
             void ensureTempBufferSize(size_t len);
             
             auto constexpr shiftRight() -> u32
               {
                  return (sizeof(TIn) - sizeof(TOut))*BITS_IN_BYTE;
               }
             
             typename Converter<TIn>::ptr _src;
             
             std::vector<u8> _tempVec;
             mods::utils::RWBuffer<u8> _temp;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_DOWNSCALECONVERTER_HPP
