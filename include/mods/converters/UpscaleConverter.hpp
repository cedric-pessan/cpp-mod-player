#ifndef MODS_CONVERTERS_UPSCALEWAVCONVERTER_HPP
#define MODS_CONVERTERS_UPSCALEWAVCONVERTER_HPP

#include "mods/converters/Converter.hpp"

namespace mods
{
   namespace converters
     {
        template<typename TOut, typename TIn>
          class UpscaleConverter : public mods::converters::Converter
          {
           public:
             explicit UpscaleConverter(Converter::ptr src);
             
             UpscaleConverter() = delete;
             UpscaleConverter(const UpscaleConverter&) = delete;
             UpscaleConverter(UpscaleConverter&&) = delete;
             auto operator=(const UpscaleConverter&) -> UpscaleConverter& = delete;
             auto operator=(UpscaleConverter&&) -> UpscaleConverter& = delete;
             ~UpscaleConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             constexpr auto shiftLeftValue() -> u32
               {
                  return sizeof(TOut)*BITS_IN_BYTE - sizeof(TIn)*BITS_IN_BYTE;
               }
             
             constexpr auto shiftRightValue() -> u32
               {
                  return sizeof(TIn)*BITS_IN_BYTE - shiftLeftValue();
               }
             
             constexpr auto maskValue() -> u32
               {
                  return (1U << shiftLeftValue()) - 1;
               }
             
             Converter::ptr _src;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_UPSCALEWAVCONVERTER_HPP
