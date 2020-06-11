#ifndef MODS_WAV_IMPL_DVIADPCMDECODERCONVERTERIMPL_HPP
#define MODS_WAV_IMPL_DVIADPCMDECODERCONVERTERIMPL_HPP

namespace mods
{
   namespace wav
     {
        namespace impl
          {
#pragma pack(push,1)
             struct DVIADPCMHeader
               {
                private:
                  s16le firstSample;
                  u8 stepSizeTableIndex;
                  u8 reserved;
                  
                public:
                  auto getFirstSample() const noexcept -> s16
                    {
                       return static_cast<s16>(firstSample);
                    }
                  
                  auto getStepSizeTableIndex() const noexcept -> u8
                    {
                       return stepSizeTableIndex;
                    }
               };
#pragma pack(pop)
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_DVIADPCMDECODERCONVERTERIMPL_HPP
