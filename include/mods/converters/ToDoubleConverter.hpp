#ifndef MODS_CONVERTERS_TODOUBLECONVERTER_HPP
#define MODS_CONVERTERS_TODOUBLECONVERTER_HPP

#include "mods/converters/Converter.hpp"

namespace mods
{
   namespace converters
     {
        template<typename T>
          class ToDoubleConverter : public mods::converters::Converter
          {
           public:
             explicit ToDoubleConverter(Converter::ptr src);
             
             ToDoubleConverter() = delete;
             ToDoubleConverter(const ToDoubleConverter&) = delete;
             ToDoubleConverter(ToDoubleConverter&&) = delete;
             auto operator=(const ToDoubleConverter&) -> ToDoubleConverter& = delete;
             auto operator=(ToDoubleConverter&&) -> ToDoubleConverter& = delete;
             ~ToDoubleConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             auto convert(T in) -> double;
             
             Converter::ptr _src;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_TODOUBLECONVERTER_HPP
