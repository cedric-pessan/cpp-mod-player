#ifndef MODS_CONVERTERS_CONVERTER_HPP
#define MODS_CONVERTERS_CONVERTER_HPP

#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"

namespace mods
{
   namespace converters
     {
        class Converter
          {
           public:
             using ptr = std::unique_ptr<Converter>;
             
             Converter() = default;
             virtual ~Converter() = default;
             
             virtual auto isFinished() const -> bool = 0;
             virtual void read(mods::utils::RWBuffer<u8>* buf, size_t len) = 0;
             
             Converter(const Converter&) = delete;
             Converter(Converter&&) = delete;
             auto operator=(const Converter&) -> Converter& = delete;
             auto operator=(Converter&&) -> Converter& = delete;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_CONVERTER_HPP
