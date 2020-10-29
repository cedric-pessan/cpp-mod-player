#ifndef MODS_WAV_OKIADPCMDECODERCONVERTER_HPP
#define MODS_WAV_OKIADPCMDECODERCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
	class OKIADPCMDecoderConverter : public WavConverter
	  {
	   public:
	     OKIADPCMDecoderConverter(WavConverter::ptr src, const Format& format);
	     
	     OKIADPCMDecoderConverter() = delete;
	     OKIADPCMDecoderConverter(const OKIADPCMDecoderConverter&) = delete;
	     OKIADPCMDecoderConverter(OKIADPCMDecoderConverter&&) = delete;
	     auto operator=(const OKIADPCMDecoderConverter&) -> OKIADPCMDecoderConverter& = delete;
	     auto operator=(OKIADPCMDecoderConverter&&) -> OKIADPCMDecoderConverter& = delete;
	     ~OKIADPCMDecoderConverter() override = default;
	     
	     auto isFinished() const -> bool override;
	     void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
             static constexpr auto getOutputBitsPerSample() -> int
               {
                  constexpr int outputBitsPerSample = 12;
                  return outputBitsPerSample;
               }
	     
	   private:
	     auto allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>;
             void ensureTempBufferSize(size_t len);
	     
	     WavConverter::ptr _src;
             u16 _nbChannels;
	     
	     std::vector<u8> _encodedVec;
             mods::utils::RWBuffer<u8> _encodedBuffer;
             
             class Decoder
               {
                public:
                  Decoder(mods::utils::RBuffer<u8> encodedBuffer,
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
                  
                  void resetBuffer(mods::utils::RBuffer<u8> encodedBuffer);
                  
                  auto decodeSample(int sample) -> s16;
                  
                private:
                  bool _sampleAvailable = false;
                  mods::utils::RBuffer<u8> _encodedBuffer;
                  mods::utils::RBuffer<u8>::const_iterator _itEncodedBuffer;
                  int _currentByteInDataWord = 0;
                  int _nbChannels;
                  int _numChannel;
                  
                  int _index = 16;
                  u32 _stepSize;
                  
                  int _newSample = 0;
                  
                  s16 _nextSample = 0;
               };
             
             std::vector<Decoder> _decoders;
             size_t _currentChannel = 0;
	  };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_OKIADPCMDECODERCONVERTER_HPP
