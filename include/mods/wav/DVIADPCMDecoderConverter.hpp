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
	     DVIADPCMDecoderConverter(WavConverter::ptr src);
	     
	     DVIADPCMDecoderConverter() = delete;
	     DVIADPCMDecoderConverter(const DVIADPCMDecoderConverter&) = delete;
	     DVIADPCMDecoderConverter(DVIADPCMDecoderConverter&&) = delete;
	     DVIADPCMDecoderConverter& operator=(const DVIADPCMDecoderConverter&) = delete;
	     DVIADPCMDecoderConverter& operator=(DVIADPCMDecoderConverter&&) = delete;
	     ~DVIADPCMDecoderConverter() = default;
	     
	     bool isFinished() const override;
	     void read(mods::utils::RWBuffer<u8>* buf, int len) override;
	     
	   private:
	     mods::utils::RWBuffer<u8> allocateNewTempBuffer(size_t len);
	     void ensureTempBufferSize(size_t len);
	     s16 decodeSample(int sample);
	     
	     WavConverter::ptr _src;
	     
	     bool _sampleAvailable = false;
	     int _sample = 0;
	     int _stepSize = 7;
	     int _newSample = 0;
	     int _index = 0;
	     
	     std::vector<u8> _tempVec;
             mods::utils::RWBuffer<u8> _temp;
	  };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_DVIADPCMDECODERCONVERTER_HPP