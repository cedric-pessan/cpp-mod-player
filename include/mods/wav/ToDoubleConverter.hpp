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
             auto operator=(const ToDoubleConverter&) -> ToDoubleConverter& = delete;
             auto operator=(ToDoubleConverter&&) -> ToDoubleConverter& = delete;
             ~ToDoubleConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             auto convert(T in) -> double;
             
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_TODOUBLECONVERTER_HPP
