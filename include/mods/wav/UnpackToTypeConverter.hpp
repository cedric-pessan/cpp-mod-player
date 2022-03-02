#ifndef MODS_WAV_UNPACKTOTYPECONVERTER_HPP
#define MODS_WAV_UNPACKTOTYPECONVERTER_HPP

#include "mods/converters/Converter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename T>
          class UnpackToTypeConverter : public mods::converters::Converter
          {
           public:
             explicit UnpackToTypeConverter(Converter::ptr src, size_t packSize);
             
             UnpackToTypeConverter() = delete;
             UnpackToTypeConverter(const UnpackToTypeConverter&) = delete;
             UnpackToTypeConverter(UnpackToTypeConverter&&) = delete;
             auto operator=(const UnpackToTypeConverter&) -> UnpackToTypeConverter& = delete;
             auto operator=(UnpackToTypeConverter&&) -> UnpackToTypeConverter& = delete;
             ~UnpackToTypeConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             Converter::ptr _src;
             size_t _packSize;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_UNPACKTOTYPECONVERTER_HPP
