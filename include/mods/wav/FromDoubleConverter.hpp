#ifndef MODS_WAV_FROMDOUBLECONVERTER_HPP
#define MODS_WAV_FROMDOUBLECONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class FromDoubleConverter : public WavConverter
          {
           public:
             FromDoubleConverter(WavConverter::ptr src);
             
             FromDoubleConverter() = delete;
             FromDoubleConverter(const FromDoubleConverter&) = delete;
             FromDoubleConverter(FromDoubleConverter&&) = delete;
             FromDoubleConverter& operator=(const FromDoubleConverter&) = delete;
             FromDoubleConverter& operator=(FromDoubleConverter&&) = delete;
             ~FromDoubleConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_FROMDOUBLECONVERTER_HPP
