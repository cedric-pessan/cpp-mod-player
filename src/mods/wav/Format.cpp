
#include "mods/wav/Format.hpp"

namespace mods
{
   namespace wav
     {
        Format::Format(mods::utils::RBuffer<FmtHeader>&& fmtHeader,
                       optional<mods::utils::RBuffer<ExtensibleHeader>>&& extensibleHeader,
                       bool useChannelMask,
                       optional<mods::utils::RBuffer<u8>>&& metadataExtension)
          : _fmtHeader(std::move(fmtHeader)),
          _extensibleHeader(std::move(extensibleHeader)),
          _useChannelMask(useChannelMask),
          _metadataExtension(std::move(metadataExtension))
            {
            }
        
        auto Format::getAudioFormat() const noexcept -> WavAudioFormat
          {
             auto fmt = _fmtHeader->getAudioFormat();
             if(fmt != WavAudioFormat::EXTENSIBLE || !_extensibleHeader.has_value())
               {
                  return fmt;
               }
             
             return (*_extensibleHeader)->getAudioFormat();
          }
        
        auto Format::getAudioFormatAsNumber() const noexcept -> u16
          {
             auto fmt = _fmtHeader->getAudioFormat();
             if(fmt != WavAudioFormat::EXTENSIBLE || !_extensibleHeader.has_value())
               {
                  return _fmtHeader->getAudioFormatAsNumber();
               }
             
             return (*_extensibleHeader)->getAudioFormatAsNumber();
          }
        
        auto Format::getNumChannels() const noexcept -> u16
          {
             return _fmtHeader->getNumChannels();
          }
        
        auto Format::getSampleRate() const noexcept -> u32
          {
             return _fmtHeader->getSampleRate();
          }
        
        auto Format::getBitsPerSample() const noexcept -> u16
          {
             if(_extensibleHeader.has_value())
               {
                  const auto& extensibleHeader = *_extensibleHeader;
                  return extensibleHeader->getValidBitsPerSample();
               }
             return _fmtHeader->getBitsPerSample();
          }
        
        auto Format::getBitsPerContainer() const noexcept -> u16
          {
             if(_extensibleHeader.has_value())
               {
                  return _fmtHeader->getBitsPerSample();
               }
             return _fmtHeader->getBlockAlign() * BITS_IN_BYTE / getNumChannels();
          }
        
        auto Format::getChannelMask() const noexcept -> u32
          {
             if(_useChannelMask && _extensibleHeader.has_value())
               {
                  const auto& extensibleHeader = *_extensibleHeader;
                  return extensibleHeader->getChannelMask();
               }
             return 0;
          }
        
        auto Format::hasMetaData() const noexcept -> bool
          {
             return _metadataExtension.has_value();
          }
        
        auto Format::getMetaData() const noexcept -> mods::utils::RBuffer<u8>
          {
             if(!_metadataExtension.has_value())
               {
                  auto deleter = std::make_unique<mods::utils::RBufferBackend::EmptyDeleter>();
                  auto backend = std::make_unique<mods::utils::RBufferBackend>(nullptr, 0, std::move(deleter));
                  return mods::utils::RBuffer<u8>(std::move(backend));
               }
             return *_metadataExtension;
          }
     } // namespace wav
} // namespace mods