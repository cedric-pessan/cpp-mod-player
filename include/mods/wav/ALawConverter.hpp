#ifndef MODS_WAV_ALAWCONVERTER_HPP
#define MODS_WAV_ALAWCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename T>
          class ALawConverter : public WavConverter
          {
           public:
             explicit ALawConverter(WavConverter::ptr src);
             
             ALawConverter() = delete;
             ALawConverter(const ALawConverter&) = delete;
             ALawConverter(ALawConverter&&) = delete;
             ALawConverter& operator=(const ALawConverter&) = delete;
             ALawConverter& operator=(ALawConverter&&) = delete;
             ~ALawConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
             
             
           private:
             void fillLookupTable();
             void aLawTransform(T* out, s8 value) const;
             
             WavConverter::ptr _src;
             std::array<T, 256> _lookupTable;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_ALAWCONVERTER_HPP
