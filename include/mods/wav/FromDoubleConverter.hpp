#ifndef MODS_WAV_FROMDOUBLECONVERTER_HPP
#define MODS_WAV_FROMDOUBLECONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename T>
          class FromDoubleConverter : public WavConverter
          {
           public:
             explicit FromDoubleConverter(WavConverter::ptr src);
             
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
             
             WavConverter::ptr _src;
             
             std::vector<u8> _tempVec;
             mods::utils::RWBuffer<u8> _temp;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_FROMDOUBLECONVERTER_HPP
