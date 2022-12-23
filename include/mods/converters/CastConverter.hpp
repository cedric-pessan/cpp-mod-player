#ifndef MODS_CONVERTERS_CASTCONVERTER_HPP
#define MODS_CONVERTERS_CASTCONVERTER_HPP

#include "mods/converters/Converter.hpp"

namespace mods
{
   namespace converters
     {
        template<typename TOut, typename TIn>
          class CastConverter : public mods::converters::Converter<TOut>
          {
           public:
             CastConverter(typename Converter<TIn>::ptr src);
             
             CastConverter() = delete;
             CastConverter(const CastConverter&) = delete;
             CastConverter(CastConverter&&) = delete;
             auto operator=(const CastConverter&) -> CastConverter& = delete;
             auto operator=(CastConverter&&) -> CastConverter& = delete;
             ~CastConverter() = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<TOut>* buf) override;
             
           private:
             typename Converter<TIn>::ptr _src;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_CASTCONVERTER_HPP
