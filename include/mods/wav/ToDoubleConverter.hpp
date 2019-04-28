#ifndef MODS_WAV_TODOUBLECONVERTER_HPP
#define MODS_WAV_TODOUBLECONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename T>
          class ToDoubleConverter : public WavConverter
          {
           public:
             explicit ToDoubleConverter(WavConverter::ptr src);
             
             ToDoubleConverter() = delete;
             ToDoubleConverter(const ToDoubleConverter&) = delete;
             ToDoubleConverter(ToDoubleConverter&&) = delete;
             ToDoubleConverter& operator=(const ToDoubleConverter&) = delete;
             ToDoubleConverter& operator=(ToDoubleConverter&&) = delete;
             ~ToDoubleConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_TODOUBLECONVERTER_HPP
