#ifndef MODS_WAV_DVIADPCMDECODERCONVERTER_HPP
#define MODS_WAV_DVIADPCMDECODERCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"
#include "mods/wav/impl/DVIADPCMDecoderConverterImpl.hpp"

namespace mods
{
   namespace wav
     {
	class DVIADPCMDecoderConverter : public WavConverter
	  {
	   public:
	     DVIADPCMDecoderConverter(WavConverter::ptr src, const Format& format);
	     
	     DVIADPCMDecoderConverter() = delete;
	     DVIADPCMDecoderConverter(const DVIADPCMDecoderConverter&) = delete;
	     DVIADPCMDecoderConverter(DVIADPCMDecoderConverter&&) = delete;
	     auto operator=(const DVIADPCMDecoderConverter&) -> DVIADPCMDecoderConverter& = delete;
	     auto operator=(DVIADPCMDecoderConverter&&) -> DVIADPCMDecoderConverter& = delete;
	     ~DVIADPCMDecoderConverter() override = default;
	     
	     auto isFinished() const -> bool override;
	     void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
             static constexpr auto getOutputBitsPerSample() -> int
               {
                  constexpr int outputBitsPerSample = 16;
                  return outputBitsPerSample;
               }
	     
	   private:
	     auto allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>;
	     
	     WavConverter::ptr _src;
             u32 _blockSize;
             u16 _nbChannels;
	     
	     std::vector<u8> _encodedVec;
             mods::utils::RWBuffer<u8> _encodedBuffer;
             mods::utils::RBuffer<u8> _dataBuffer;
             mods::utils::RBuffer<impl::DVIADPCMHeader> _headers;
             
             class Decoder
               {
                public:
                  Decoder(mods::utils::RBuffer<u8> dataBuffer,
                          const mods::utils::RBuffer<impl::DVIADPCMHeader>& headers,
                          int numChannel,
                          int nbChannels);
                  
                  Decoder() = delete;
                  Decoder(const Decoder&) = delete;
                  Decoder(Decoder&&) = default;
                  auto operator=(const Decoder&) -> Decoder& = delete;
                  auto operator=(Decoder&&) -> Decoder& = delete;
                  ~Decoder() = default;
                  
                  auto hasNextSampleInCurrentBlock() const -> bool;
                  auto getSample() -> s16;
                  
                  void resetBuffer();
                  
                  auto decodeSample(int sample) -> s16;
                  
                private:
                  bool _sampleAvailable = false;
                  const mods::utils::RBuffer<u8> _dataBuffer;
                  mods::utils::RBuffer<u8>::const_iterator _itDataBuffer;
                  int _currentByteInDataWord = 0;
                  bool _firstSampleInBlock = true;
                  const impl::DVIADPCMHeader& _header;
                  int _nbChannels;
                  int _numChannel;
                  
                  static constexpr u32 _defaultStepSize = 7;
                  
                  u32 _stepSize = _defaultStepSize;
                  int _index = 0;
                  
                  int _newSample = 0;
                  
                  s16 _nextSample = 0;
               };
             
             std::vector<Decoder> _decoders;
             size_t _currentChannel = 0;
	  };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_DVIADPCMDECODERCONVERTER_HPP
