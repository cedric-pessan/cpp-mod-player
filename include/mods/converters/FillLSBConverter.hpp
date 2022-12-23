#ifndef MODS_CONVERTERS_FILLLSBCONVERTER_HPP
#define MODS_CONVERTERS_FILLLSBCONVERTER_HPP

#include "mods/converters/Converter.hpp"

namespace mods
{
   namespace converters
     {
        template<typename T>
          class FillLSBConverter : public mods::converters::Converter<T>
          {
           public:
             FillLSBConverter(typename Converter<T>::ptr src, u32 bitsToFill);
             
             FillLSBConverter() = delete;
             FillLSBConverter(const FillLSBConverter&) = delete;
             FillLSBConverter(FillLSBConverter&&) = delete;
             auto operator=(const FillLSBConverter&) -> FillLSBConverter& = delete;
             auto operator=(FillLSBConverter&&) -> FillLSBConverter& = delete;
             ~FillLSBConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<T>* buf) override;
             
           private:
             typename Converter<T>::ptr _src;
             u32 _shift;
             u32 _mask;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_FILLLSBCONVERTER_HPP
