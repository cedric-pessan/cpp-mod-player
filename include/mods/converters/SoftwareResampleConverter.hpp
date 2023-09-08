#ifndef MODS_CONVERTERS_SOFTWARERESAMPLECONVERTER_HPP
#define MODS_CONVERTERS_SOFTWARERESAMPLECONVERTER_HPP

#include "mods/converters/ResampleConverter.hpp"

namespace mods
{
   namespace converters
     {
        template<typename PARAMETERS, typename T>
          class SoftwareResampleConverter : public ResampleConverter<PARAMETERS, T>
          {
           public:
             SoftwareResampleConverter(typename Converter<T>::ptr src, PARAMETERS parameters);
             
             SoftwareResampleConverter() = delete;
             SoftwareResampleConverter(const SoftwareResampleConverter&) = delete;
             SoftwareResampleConverter(SoftwareResampleConverter&&) = delete;
             auto operator=(const SoftwareResampleConverter&) -> SoftwareResampleConverter& = delete;
             auto operator=(SoftwareResampleConverter&&) -> SoftwareResampleConverter& = delete;
             ~SoftwareResampleConverter() override = default;
             
             void read(mods::utils::RWBuffer<double>* buf) override;
             
           private:
             auto getNextDecimatedSample() -> double;
             void updateHistory();
             auto calculateInterpolatedSample() -> double;
             auto getTap(size_t i, const impl::SampleWithZeros& sample) const -> double;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_SOFTWARERESAMPLECONVERTER_HPP
