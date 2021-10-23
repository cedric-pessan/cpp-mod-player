#ifndef MODS_WAV_SOFTWARERESAMPLECONVERTER_HPP
#define MODS_WAV_SOFTWARERESAMPLECONVERTER_HPP

#include "mods/wav/ResampleConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename PARAMETERS>
          class SoftwareResampleConverter : public ResampleConverter<PARAMETERS>
          {
           public:
             SoftwareResampleConverter(WavConverter::ptr src, PARAMETERS parameters);
             
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
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_SOFTWARERESAMPLECONVERTER_HPP
