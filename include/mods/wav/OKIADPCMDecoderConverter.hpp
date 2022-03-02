#ifndef MODS_WAV_OKIADPCMDECODERCONVERTER_HPP
#define MODS_WAV_OKIADPCMDECODERCONVERTER_HPP

#include "mods/converters/Converter.hpp"

#include <vector>

namespace mods
{
   namespace wav
     {
        class Format;
        
	class OKIADPCMDecoderConverter : public mods::converters::Converter
	  {
	   public:
	     OKIADPCMDecoderConverter(Converter::ptr src, const Format& format);
	     
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
	     
	     Converter::ptr _src;
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
                  
                  void resetBuffer(const mods::utils::RBuffer<u8>& encodedBuffer);
                  
                private:
                  auto decodeSample(int sample) -> s16;
                  
                  bool _sampleAvailable = false;
                  mods::utils::RBuffer<u8> _encodedBuffer;
                  mods::utils::RBuffer<u8>::const_iterator _itEncodedBuffer;
                  int _currentByteInDataWord = 0;
                  int _nbChannels;
                  int _numChannel;
                  
                  constexpr static int _initialIndex = 16;
                  int _index = _initialIndex;
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
