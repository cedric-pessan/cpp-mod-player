#ifndef MODS_WAV_BYTESSKIPCONVERTER_HPP
#define MODS_WAV_BYTESSKIPCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<int KEEP, int SKIP>
          class BytesSkipConverter : public WavConverter
          {
           public:
             explicit BytesSkipConverter(WavConverter::ptr src);
             
             BytesSkipConverter() = delete;
             BytesSkipConverter(const BytesSkipConverter&) = delete;
             BytesSkipConverter(const BytesSkipConverter&&) = delete;
             BytesSkipConverter& operator=(const BytesSkipConverter&) = delete;
             BytesSkipConverter& operator=(const BytesSkipConverter&&) = delete;
             ~BytesSkipConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             mods::utils::RWBuffer<u8> allocateNewTempBuffer(size_t len);
             void ensureTempBufferSize(size_t len);
             
             WavConverter::ptr _src;
             
             std::vector<u8> _tempVec;
             mods::utils::RWBuffer<u8> _temp;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_BYTESSKIPCONVERTER_HPP
