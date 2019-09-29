#ifndef MODS_WAV_FORMAT_HPP
#define MODS_WAV_FORMAT_HPP

#include "mods/utils/RBuffer.hpp"
#include "mods/utils/optional.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/WavTypes.hpp"

namespace mods
{
   namespace wav
     {
        class Format
          {
           public:
             Format(mods::utils::RBuffer<FmtHeader>&& fmtHeader,
                    optional<mods::utils::RBuffer<ExtensibleHeader>>&& extensibleHeader);
             
             Format() = delete;
             Format(const Format&) = delete;
             Format(Format&&) = default;
             Format& operator=(const Format&) = delete;
             Format& operator=(Format&&) = default;
             ~Format() = default;
             
             WavAudioFormat getAudioFormat() const noexcept;
             
             u16 getAudioFormatAsNumber() const noexcept;
             
             u16 getNumChannels() const noexcept;
             
             u32 getSampleRate() const noexcept;
             
             u16 getBitsPerSample() const noexcept;
             
             u16 getBitsPerContainer() const noexcept;
             
           private:
             mods::utils::RBuffer<FmtHeader> _fmtHeader;
             optional<mods::utils::RBuffer<ExtensibleHeader>> _extensibleHeader;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_FORMAT_HPP
