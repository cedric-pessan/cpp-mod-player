#ifndef MODS_CONVERTERS_DIVIDECONVERTER_HPP
#define MODS_CONVERTERS_DIVIDECONVERTER_HPP

#include "mods/converters/Converter.hpp"

namespace mods
{
   namespace converters
     {
        class DivideConverter : public Converter<double>
          {
           public:
             DivideConverter(Converter<double>::ptr src, double coef);
             
             DivideConverter() = delete;
             DivideConverter(const DivideConverter&) = delete;
             DivideConverter(DivideConverter&&) = delete;
             auto operator=(const DivideConverter&) -> DivideConverter& = delete;
             auto operator=(DivideConverter&&) -> DivideConverter& = delete;
             ~DivideConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<double>* buf) override;
             
           private:
             Converter<double>::ptr _src;
             double _coef;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_DIVIDECONVERTER_HPP
