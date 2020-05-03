#ifndef MODS_WAV_RESAMPLEPOSITIVEINTEGERFACTOR_HPP
#define MODS_WAV_RESAMPLEPOSITIVEINTEGERFACTOR_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename T, int FACTOR>
          class ResamplePositiveIntegerFactor : public WavConverter
          {
           public:
             explicit ResamplePositiveIntegerFactor(WavConverter::ptr src);
             
             ResamplePositiveIntegerFactor() = delete;
             ResamplePositiveIntegerFactor(const ResamplePositiveIntegerFactor&) = delete;
             ResamplePositiveIntegerFactor(ResamplePositiveIntegerFactor&&) = delete;
             auto operator=(const ResamplePositiveIntegerFactor&) -> ResamplePositiveIntegerFactor& = delete;
             auto operator=(ResamplePositiveIntegerFactor&&) -> ResamplePositiveIntegerFactor& = delete;
             ~ResamplePositiveIntegerFactor() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_RESAMPLEPOSITIVEINTEGERFACTOR_HPP
