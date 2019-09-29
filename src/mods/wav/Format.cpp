
#include "mods/wav/Format.hpp"

namespace mods
{
   namespace wav
     {
        Format::Format(mods::utils::RBuffer<FmtHeader>&& fmtHeader,
                       optional<mods::utils::RBuffer<ExtensibleHeader>>&& extensibleHeader)
          : _fmtHeader(std::move(fmtHeader)),
          _extensibleHeader(std::move(extensibleHeader))
            {
            }
        
        WavAudioFormat Format::getAudioFormat() const noexcept
          {
             auto fmt = _fmtHeader->getAudioFormat();
             if(fmt != WavAudioFormat::EXTENSIBLE || !_extensibleHeader.has_value())
               {
                  return fmt;
               }
             
             return (*_extensibleHeader)->getAudioFormat();
          }
        
        u16 Format::getAudioFormatAsNumber() const noexcept
          {
             auto fmt = _fmtHeader->getAudioFormat();
             if(fmt != WavAudioFormat::EXTENSIBLE || !_extensibleHeader.has_value())
               {
                  return _fmtHeader->getAudioFormatAsNumber();
               }
             
             return (*_extensibleHeader)->getAudioFormatAsNumber();
          }
        
        u16 Format::getNumChannels() const noexcept
          {
             return _fmtHeader->getNumChannels();
          }
        
        u32 Format::getSampleRate() const noexcept
          {
             return _fmtHeader->getSampleRate();
          }
        
        u16 Format::getBitsPerSample() const noexcept
          {
             if(_extensibleHeader.has_value())
               {
                  auto& extensibleHeader = *_extensibleHeader;
                  return extensibleHeader->getValidBitsPerSample();
               }
             return _fmtHeader->getBitsPerSample();
          }
        
        u16 Format::getBitsPerContainer() const noexcept
          {
             if(_extensibleHeader.has_value())
               {
                  return _fmtHeader->getBitsPerSample();
               }
             return _fmtHeader->getBlockAlign() * 8 / getNumChannels();
          }
     } // namespace wav
} // namespace mods