#ifndef MODS_CONVERTERS_MULTIPLEXERCONVERTER_HPP
#define MODS_CONVERTERS_MULTIPLEXERCONVERTER_HPP

#include "mods/converters/Converter.hpp"

#include <vector>

namespace mods
{
   namespace converters
     {
        template<typename T>
          class MultiplexerConverter : public Converter<T>
          {
           public:
             MultiplexerConverter(typename Converter<T>::ptr left, typename Converter<T>::ptr right);
             
             MultiplexerConverter() = delete;
             MultiplexerConverter(const MultiplexerConverter&) = delete;
             MultiplexerConverter(MultiplexerConverter&&) = delete;
             auto operator=(const MultiplexerConverter&) -> MultiplexerConverter& = delete;
             auto operator=(MultiplexerConverter&&) -> MultiplexerConverter& = delete;
             ~MultiplexerConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<T>* buf) override;
             
           private:
             auto allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<T>;
             void ensureTempBufferSize(size_t len);
             
             typename Converter<T>::ptr _left;
             typename Converter<T>::ptr _right;
             
             std::vector<u8> _tempVec;
             mods::utils::RWBuffer<T> _temp;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_MULTIPLEXERCONVERTER_HPP
