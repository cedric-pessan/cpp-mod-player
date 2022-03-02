#ifndef MODS_CONVERTERS_UNSIGNEDTOSIGNEDCONVERTER_HPP
#define MODS_CONVERTERS_UNSIGNEDTOSIGNEDCONVERTER_HPP

#include "mods/converters/Converter.hpp"

namespace mods
{
   namespace converters
     {
        template<typename T>
          class UnsignedToSignedConverter : public Converter
          {
           public:
             explicit UnsignedToSignedConverter(Converter::ptr src);
             
             UnsignedToSignedConverter() = delete;
             UnsignedToSignedConverter(const UnsignedToSignedConverter&) = delete;
             UnsignedToSignedConverter(UnsignedToSignedConverter&&) = delete;
             auto operator=(const UnsignedToSignedConverter&) -> UnsignedToSignedConverter& = delete;
             auto operator=(UnsignedToSignedConverter&&) -> UnsignedToSignedConverter& = delete;
             ~UnsignedToSignedConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             constexpr auto getOffset() -> int
               {
                  return (1U << (sizeof(T) * BITS_IN_BYTE)) / 2;
               }
             
             Converter::ptr _src;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_UNSIGNEDTOSIGNEDCONVERTER_HPP
