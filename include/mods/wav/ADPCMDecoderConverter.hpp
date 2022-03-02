#ifndef MODS_WAV_ADPCMDECODERCONVERTER_HPP
#define MODS_WAV_ADPCMDECODERCONVERTER_HPP

#include "mods/converters/Converter.hpp"
#include "mods/wav/impl/ADPCMDecoderConverterImpl.hpp"

#include <vector>

namespace mods
{
   namespace wav
     {
        class Format;
        
        template<int NB_CHANNELS>
          class ADPCMDecoderConverter : public mods::converters::Converter
	  {
	   public:
	     ADPCMDecoderConverter(Converter::ptr src, const Format& format);
	     
	     ADPCMDecoderConverter() = delete;
	     ADPCMDecoderConverter(const ADPCMDecoderConverter&) = delete;
             ADPCMDecoderConverter(ADPCMDecoderConverter&&) = delete;
	     auto operator=(const ADPCMDecoderConverter&) -> ADPCMDecoderConverter& = delete;
	     auto operator=(ADPCMDecoderConverter&&) -> ADPCMDecoderConverter& = delete;
	     ~ADPCMDecoderConverter() override = default;
	     
	     auto isFinished() const -> bool override;
	     void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
             static constexpr auto getOutputBitsPerSample() -> int
               {
                  constexpr int outputBitsPerSample = 16;
                  return outputBitsPerSample;
               }
             
             static constexpr auto isExtensionSizeValid(size_t size) -> bool
               {
                  return size >= sizeof(impl::ADPCMExtension);
               }
	     
	   private:
             using Preamble = impl::ADPCMPreamble<NB_CHANNELS>;
             
             auto allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>;
             auto initBlockSize() -> u32;
	     
	     Converter::ptr _src;
	     
             bool _sampleAvailable = false;
	     s16 _nextSample = 0;
             
             mods::utils::RBuffer<impl::ADPCMExtension> _extension;
             mods::utils::RBuffer<s16> _coefs;
             u32 _blockSize;
             
             std::vector<u8> _encodedVec;
             mods::utils::RWBuffer<u8> _encodedBuffer;
             mods::utils::RBuffer<u8> _dataBuffer;
             mods::utils::RBuffer<u8>::const_iterator _itDataBuffer;
             mods::utils::RBuffer<Preamble> _preamble;
	     
	     std::array<impl::ADPCMChannelDecoder, NB_CHANNELS> _decoders;
	  };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_ADPCMDECODERCONVERTER_HPP
