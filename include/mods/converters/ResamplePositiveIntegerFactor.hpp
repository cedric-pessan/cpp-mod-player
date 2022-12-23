#ifndef MODS_CONVERTERS_RESAMPLEPOSITIVEINTEGERFACTOR_HPP
#define MODS_CONVERTERS_RESAMPLEPOSITIVEINTEGERFACTOR_HPP

#include "mods/converters/Converter.hpp"

namespace mods
{
   namespace converters
     {
        template<typename T, int FACTOR>
          class ResamplePositiveIntegerFactor : public Converter<T>
          {
           public:
             explicit ResamplePositiveIntegerFactor(typename Converter<T>::ptr src);
             
             ResamplePositiveIntegerFactor() = delete;
             ResamplePositiveIntegerFactor(const ResamplePositiveIntegerFactor&) = delete;
             ResamplePositiveIntegerFactor(ResamplePositiveIntegerFactor&&) = delete;
             auto operator=(const ResamplePositiveIntegerFactor&) -> ResamplePositiveIntegerFactor& = delete;
             auto operator=(ResamplePositiveIntegerFactor&&) -> ResamplePositiveIntegerFactor& = delete;
             ~ResamplePositiveIntegerFactor() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<T>* buf) override;
             
           private:
             typename Converter<T>::ptr _src;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_RESAMPLEPOSITIVEINTEGERFACTOR_HPP
