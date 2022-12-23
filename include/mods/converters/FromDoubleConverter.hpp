#ifndef MODS_CONVERTERS_FROMDOUBLECONVERTER_HPP
#define MODS_CONVERTERS_FROMDOUBLECONVERTER_HPP

#include "mods/converters/Converter.hpp"

#include <vector>

namespace mods
{
   namespace converters
     {
        template<typename T>
          class FromDoubleConverter : public Converter<T>
          {
           public:
             explicit FromDoubleConverter(Converter<double>::ptr src);
             
             FromDoubleConverter() = delete;
             FromDoubleConverter(const FromDoubleConverter&) = delete;
             FromDoubleConverter(FromDoubleConverter&&) = delete;
             auto operator=(const FromDoubleConverter&) -> FromDoubleConverter& = delete;
             auto operator=(FromDoubleConverter&&) -> FromDoubleConverter& = delete;
             ~FromDoubleConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<T>* buf) override;
             
           private:
             auto allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<double>;
             void ensureTempBufferSize(size_t len);
             
             Converter<double>::ptr _src;
             
             std::vector<u8> _tempVec;
             mods::utils::RWBuffer<double> _temp;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_FROMDOUBLECONVERTER_HPP
