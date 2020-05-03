#ifndef MODS_WAV_FILLLSBCONVERTER_HPP
#define MODS_WAV_FILLLSBCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename T>
          class FillLSBConverter : public WavConverter
          {
           public:
             FillLSBConverter(WavConverter::ptr src, u32 bitsToFill);
             
             FillLSBConverter() = delete;
             FillLSBConverter(const FillLSBConverter&) = delete;
             FillLSBConverter(FillLSBConverter&&) = delete;
             auto operator=(const FillLSBConverter&) -> FillLSBConverter& = delete;
             auto operator=(FillLSBConverter&&) -> FillLSBConverter& = delete;
             ~FillLSBConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             WavConverter::ptr _src;
             u32 _shift;
             u32 _mask;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_FILLLSBCONVERTER_HPP
