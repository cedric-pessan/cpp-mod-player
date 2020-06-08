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
                    optional<mods::utils::RBuffer<ExtensibleHeader>>&& extensibleHeader,
                    bool useChannelMask,
                    optional<mods::utils::RBuffer<u8>>&& metadataExtension);
             
             Format() = delete;
             Format(const Format&) = delete;
             Format(Format&&) = default;
             auto operator=(const Format&) -> Format& = delete;
             auto operator=(Format&&) -> Format& = default;
             ~Format() = default;
             
             auto getAudioFormat() const noexcept -> WavAudioFormat;
             
             auto getAudioFormatAsNumber() const noexcept -> u16;
             
             auto getNumChannels() const noexcept -> u16;
             
             auto getSampleRate() const noexcept -> u32;
             
             auto getBitsPerSample() const noexcept -> u16;
             
             auto getBitsPerContainer() const noexcept -> u16;
             
             auto getChannelMask() const noexcept -> u32;
             
           private:
             mods::utils::RBuffer<FmtHeader> _fmtHeader;
             optional<mods::utils::RBuffer<ExtensibleHeader>> _extensibleHeader;
             bool _useChannelMask;
             optional<mods::utils::RBuffer<u8>> _metadataExtension;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_FORMAT_HPP
