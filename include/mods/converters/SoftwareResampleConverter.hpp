#ifndef MODS_CONVERTERS_SOFTWARERESAMPLECONVERTER_HPP
#define MODS_CONVERTERS_SOFTWARERESAMPLECONVERTER_HPP

#include "mods/converters/ResampleConverter.hpp"

namespace mods
{
   namespace converters
     {
        template<typename PARAMETERS>
          class SoftwareResampleConverter : public ResampleConverter<PARAMETERS>
          {
           public:
             SoftwareResampleConverter(Converter::ptr src, PARAMETERS parameters);
             
             SoftwareResampleConverter() = delete;
             SoftwareResampleConverter(const SoftwareResampleConverter&) = delete;
             SoftwareResampleConverter(SoftwareResampleConverter&&) = delete;
             auto operator=(const SoftwareResampleConverter&) -> SoftwareResampleConverter& = delete;
             auto operator=(SoftwareResampleConverter&&) -> SoftwareResampleConverter& = delete;
             ~SoftwareResampleConverter() override = default;
             
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             auto getNextDecimatedSample() -> double;
             void updateHistory();
             auto calculateInterpolatedSample() -> double;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_SOFTWARERESAMPLECONVERTER_HPP
