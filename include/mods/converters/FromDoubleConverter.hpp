#ifndef MODS_CONVERTERS_FROMDOUBLECONVERTER_HPP
#define MODS_CONVERTERS_FROMDOUBLECONVERTER_HPP

#include "mods/converters/Converter.hpp"

#include <vector>

namespace mods
{
   namespace converters
     {
        template<typename T>
          class FromDoubleConverter : public Converter
          {
           public:
             explicit FromDoubleConverter(Converter::ptr src);
             
             FromDoubleConverter() = delete;
             FromDoubleConverter(const FromDoubleConverter&) = delete;
             FromDoubleConverter(FromDoubleConverter&&) = delete;
             auto operator=(const FromDoubleConverter&) -> FromDoubleConverter& = delete;
             auto operator=(FromDoubleConverter&&) -> FromDoubleConverter& = delete;
             ~FromDoubleConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             auto allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>;
             void ensureTempBufferSize(size_t len);
             
             Converter::ptr _src;
             
             std::vector<u8> _tempVec;
             mods::utils::RWBuffer<u8> _temp;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_FROMDOUBLECONVERTER_HPP
