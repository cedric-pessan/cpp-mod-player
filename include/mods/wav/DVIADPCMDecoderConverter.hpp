#ifndef MODS_WAV_DVIADPCMDECODERCONVERTER_HPP
#define MODS_WAV_DVIADPCMDECODERCONVERTER_HPP

#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
	class DVIADPCMDecoderConverter : public WavConverter
	  {
	   public:
	     explicit DVIADPCMDecoderConverter(WavConverter::ptr src);
	     
	     DVIADPCMDecoderConverter() = delete;
	     DVIADPCMDecoderConverter(const DVIADPCMDecoderConverter&) = delete;
	     DVIADPCMDecoderConverter(DVIADPCMDecoderConverter&&) = delete;
	     auto operator=(const DVIADPCMDecoderConverter&) -> DVIADPCMDecoderConverter& = delete;
	     auto operator=(DVIADPCMDecoderConverter&&) -> DVIADPCMDecoderConverter& = delete;
	     ~DVIADPCMDecoderConverter() override = default;
	     
	     auto isFinished() const -> bool override;
	     void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
	     
	   private:
	     auto allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>;
	     void ensureTempBufferSize(size_t len);
	     auto decodeSample(int sample) -> s16;
	     
	     WavConverter::ptr _src;
             
             static constexpr u32 _defaultStepSize = 7;
	     
	     bool _sampleAvailable = false;
	     int _sample = 0;
	     u32 _stepSize = _defaultStepSize;
	     int _newSample = 0;
	     int _index = 0;
	     
	     std::vector<u8> _tempVec;
             mods::utils::RWBuffer<u8> _temp;
	  };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_DVIADPCMDECODERCONVERTER_HPP