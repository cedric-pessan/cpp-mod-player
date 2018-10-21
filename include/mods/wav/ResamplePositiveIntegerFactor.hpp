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
             ResamplePositiveIntegerFactor(const ResamplePositiveIntegerFactor&&) = delete;
             ResamplePositiveIntegerFactor& operator=(const ResamplePositiveIntegerFactor&) = delete;
             ResamplePositiveIntegerFactor& operator=(const ResamplePositiveIntegerFactor&&) = delete;
             ~ResamplePositiveIntegerFactor() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_RESAMPLEPOSITIVEINTEGERFACTOR_HPP
