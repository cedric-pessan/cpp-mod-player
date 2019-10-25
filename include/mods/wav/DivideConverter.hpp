#ifndef MODS_WAV_DIVIDECONVERTER_HPP
#define MODS_WAV_DIVIDECONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class DivideConverter : public WavConverter
          {
           public:
             DivideConverter(WavConverter::ptr src, double coef);
             
             DivideConverter() = delete;
             DivideConverter(const DivideConverter&) = delete;
             DivideConverter(DivideConverter&&) = delete;
             DivideConverter& operator=(const DivideConverter&) = delete;
             DivideConverter& operator=(DivideConverter&&) = delete;
             ~DivideConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             WavConverter::ptr _src;
             double _coef;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_DIVIDECONVERTER_HPP
