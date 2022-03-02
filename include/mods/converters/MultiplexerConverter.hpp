#ifndef MODS_CONVERTERS_MULTIPLEXERCONVERTER_HPP
#define MODS_CONVERTERS_MULTIPLEXERCONVERTER_HPP

#include "mods/converters/Converter.hpp"

#include <vector>

namespace mods
{
   namespace converters
     {
        class MultiplexerConverter : public Converter
          {
           public:
             MultiplexerConverter(Converter::ptr left, Converter::ptr right);
             
             MultiplexerConverter() = delete;
             MultiplexerConverter(const MultiplexerConverter&) = delete;
             MultiplexerConverter(MultiplexerConverter&&) = delete;
             auto operator=(const MultiplexerConverter&) -> MultiplexerConverter& = delete;
             auto operator=(MultiplexerConverter&&) -> MultiplexerConverter& = delete;
             ~MultiplexerConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             auto allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>;
             void ensureTempBufferSize(size_t len);
             
             Converter::ptr _left;
             Converter::ptr _right;
             
             std::vector<u8> _tempVec;
             mods::utils::RWBuffer<u8> _temp;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_MULTIPLEXERCONVERTER_HPP
