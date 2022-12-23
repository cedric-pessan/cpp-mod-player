#ifndef MODS_CONVERTERS_TODOUBLECONVERTER_HPP
#define MODS_CONVERTERS_TODOUBLECONVERTER_HPP

#include "mods/converters/Converter.hpp"

namespace mods
{
   namespace converters
     {
        template<typename T>
          class ToDoubleConverter : public mods::converters::Converter<double>
          {
           public:
             explicit ToDoubleConverter(typename Converter<T>::ptr src);
             
             ToDoubleConverter() = delete;
             ToDoubleConverter(const ToDoubleConverter&) = delete;
             ToDoubleConverter(ToDoubleConverter&&) = delete;
             auto operator=(const ToDoubleConverter&) -> ToDoubleConverter& = delete;
             auto operator=(ToDoubleConverter&&) -> ToDoubleConverter& = delete;
             ~ToDoubleConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<double>* buf) override;
             
           private:
             auto convert(T in) -> double;
             
             typename Converter<T>::ptr _src;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_TODOUBLECONVERTER_HPP
