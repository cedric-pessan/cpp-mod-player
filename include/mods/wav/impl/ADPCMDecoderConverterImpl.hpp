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
                  s16le initialDelta;
                  s16le sample1;
                  s16le sample2;
                  
                public:
                  auto getBlockPredictor() const noexcept -> u8
                    {
                       return blockPredictor;
                    }
                  
                  auto getSample1() const noexcept -> s16
                    {
                       return static_cast<s16>(sample1);
                    }
                  
                  auto getSample2() const noexcept -> s16
                    {
                       return static_cast<s16>(sample2);
                    }
                  
                  auto getInitialDelta() const noexcept -> s16
                    {
                       return static_cast<s16>(initialDelta);
                    }
               };
             
             struct ADPCMExtension
               {
                private:
                  u16le samplesPerBlock;
                  u16le numCoef;
                  
                public:
                  auto getSamplesPerBlock() const noexcept -> u16
                    {
                       return static_cast<u16>(samplesPerBlock);
                    }
                  
                  auto getNumCoef() const noexcept -> u16
                    {
                       return static_cast<u16>(numCoef);
                    }
               };
#pragma pack(pop)
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_ADPCMDECODERCONVERTERIMPL_HPP
