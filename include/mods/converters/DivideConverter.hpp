#ifndef MODS_CONVERTERS_DIVIDECONVERTER_HPP
#define MODS_CONVERTERS_DIVIDECONVERTER_HPP

#include "mods/converters/Converter.hpp"

namespace mods
{
   namespace converters
     {
        class DivideConverter : public Converter
          {
           public:
             DivideConverter(Converter::ptr src, double coef);
             
             DivideConverter() = delete;
             DivideConverter(const DivideConverter&) = delete;
             DivideConverter(DivideConverter&&) = delete;
             auto operator=(const DivideConverter&) -> DivideConverter& = delete;
             auto operator=(DivideConverter&&) -> DivideConverter& = delete;
             ~DivideConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             Converter::ptr _src;
             double _coef;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_DIVIDECONVERTER_HPP
