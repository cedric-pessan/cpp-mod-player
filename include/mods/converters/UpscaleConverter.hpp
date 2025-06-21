#ifndef MODS_CONVERTERS_UPSCALECONVERTER_HPP
#define MODS_CONVERTERS_UPSCALECONVERTER_HPP

#include "mods/converters/Converter.hpp"

namespace mods
{
   namespace converters
     {
        template<typename TOut, typename TIn>
          class UpscaleConverter : public mods::converters::Converter<TOut>
          {
           public:
             explicit UpscaleConverter(typename Converter<TIn>::ptr src);
             
             UpscaleConverter() = delete;
             UpscaleConverter(const UpscaleConverter&) = delete;
             UpscaleConverter(UpscaleConverter&&) = delete;
             auto operator=(const UpscaleConverter&) -> UpscaleConverter& = delete;
             auto operator=(UpscaleConverter&&) -> UpscaleConverter& = delete;
             ~UpscaleConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<TOut>* buf) override;
             
           private:
             constexpr auto shiftLeftValue() -> u32
               {
                  return (sizeof(TOut)*BITS_IN_BYTE) - (sizeof(TIn)*BITS_IN_BYTE);
               }
             
             constexpr auto shiftRightValue() -> u32
               {
                  return (sizeof(TIn)*BITS_IN_BYTE) - shiftLeftValue();
               }
             
             constexpr auto maskValue() -> u32
               {
                  return (1U << shiftLeftValue()) - 1;
               }
             
             typename Converter<TIn>::ptr _src;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_UPSCALECONVERTER_HPP
