#ifndef MODS_WAV_ALAWCONVERTER_HPP
#define MODS_WAV_ALAWCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

#include <type_traits>

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
             
             template<typename T2>
               typename std::enable_if<std::is_same<T2, T>::value && std::is_floating_point<T2>::value>::type aLawTransform(T2* out, s8 value) const;
             
           private:
             WavConverter::ptr _src;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_ALAWCONVERTER_HPP
