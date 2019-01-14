#ifndef MODS_WAV_RESAMPLECONVERTER_HPP
#define MODS_WAV_RESAMPLECONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class ResampleConverter : public WavConverter
          {
           public:
             explicit ResampleConverter(WavConverter::ptr src);
             
             ResampleConverter() = delete;
             ResampleConverter(const ResampleConverter&) = delete;
             ResampleConverter(ResampleConverter&&) = delete;
             ResampleConverter& operator=(const ResampleConverter&) = delete;
             ResampleConverter& operator=(ResampleConverter&&) = delete;
             ~ResampleConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             double getNextDecimatedSample();
             
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_RESAMPLECONVERTER_HPP
