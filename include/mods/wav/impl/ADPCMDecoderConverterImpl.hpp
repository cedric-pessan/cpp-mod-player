#ifndef MODS_WAV_ADPCMDECODERCONVERTERIMPL_HPP
#define MODS_WAV_ADPCMDECODERCONVERTERIMPL_HPP

#include "mods/wav/WavConverter.hpp"
#include "mods/wav/impl/ADPCMDecoderConverterImpl.hpp"

namespace mods
{
   namespace wav
     {
        namespace impl
          {
#pragma pack(push,1)
             struct ADPCMPreamble
               {
                private:
                  u8 blockPredictor;
                  u16le initialDelta;
                  u16le sample1;
                  u16le sample2;
                  
                public:
                  auto getSample1() const noexcept -> u16
                    {
                       return static_cast<u16>(sample1);
                    }
                  
                  auto getSample2() const noexcept -> u16
                    {
                       return static_cast<u16>(sample2);
                    }
               };
#pragma pack(pop)
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_ADPCMDECODERCONVERTERIMPL_HPP
