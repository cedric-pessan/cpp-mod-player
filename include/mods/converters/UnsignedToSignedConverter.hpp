#ifndef MODS_CONVERTERS_UNSIGNEDTOSIGNEDCONVERTER_HPP
#define MODS_CONVERTERS_UNSIGNEDTOSIGNEDCONVERTER_HPP

#include "mods/converters/Converter.hpp"

namespace mods
{
   namespace converters
     {
        template<typename TOut, typename TIn>
          class UnsignedToSignedConverter : public Converter<TOut>
          {
           public:
             explicit UnsignedToSignedConverter(typename Converter<TIn>::ptr src);
             
             UnsignedToSignedConverter() = delete;
             UnsignedToSignedConverter(const UnsignedToSignedConverter&) = delete;
             UnsignedToSignedConverter(UnsignedToSignedConverter&&) = delete;
             auto operator=(const UnsignedToSignedConverter&) -> UnsignedToSignedConverter& = delete;
             auto operator=(UnsignedToSignedConverter&&) -> UnsignedToSignedConverter& = delete;
             ~UnsignedToSignedConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<TOut>* buf) override;
             
           private:
             constexpr auto getOffset() -> int
               {
                  return (1U << (sizeof(TIn) * BITS_IN_BYTE)) / 2;
               }
             
             typename Converter<TIn>::ptr _src;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_UNSIGNEDTOSIGNEDCONVERTER_HPP
