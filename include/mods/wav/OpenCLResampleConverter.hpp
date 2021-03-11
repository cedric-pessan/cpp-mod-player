#ifndef MODS_WAV_OPENCLRESAMPLECONVERTER_HPP
#define MODS_WAV_OPENCLRESAMPLECONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename PARAMETERS>
          class OpenCLResampleConverter : public WavConverter
          {
           public:
             OpenCLResampleConverter(WavConverter::ptr src, PARAMETERS parameters);
             
             OpenCLResampleConverter() = delete;
             OpenCLResampleConverter(const OpenCLResampleConverter&) = delete;
             OpenCLResampleConverter(OpenCLResampleConverter&&) = delete;
             auto operator=(const OpenCLResampleConverter&) -> OpenCLResampleConverter& = delete;
             auto operator=(OpenCLResampleConverter&&) -> OpenCLResampleConverter& = delete;
             ~OpenCLResampleConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_OPENCLRESAMPLECONVERTER_HPP
