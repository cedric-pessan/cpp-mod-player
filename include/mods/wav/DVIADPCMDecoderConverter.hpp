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
	     DVIADPCMDecoderConverter(WavConverter::ptr src, u32 bitsPerContainer);
	     
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
	     auto decodeSample(int sample) -> s16;
	     
	     WavConverter::ptr _src;
             u32 _blockSize;
             
             static constexpr u32 _defaultStepSize = 7;
	     
	     bool _sampleAvailable = false;
	     u32 _stepSize = _defaultStepSize;
	     s16 _nextSample = 0;
             int _newSample = 0;
	     int _index = 0;
	     
	     std::vector<u8> _encodedVec;
             mods::utils::RWBuffer<u8> _encodedBuffer;
             mods::utils::RWBuffer<u8> _dataBuffer;
             mods::utils::RWBuffer<u8>::const_iterator _itDataBuffer;
             mods::utils::RWBuffer<impl::DVIADPCMHeader> _header;
	  };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_DVIADPCMDECODERCONVERTER_HPP