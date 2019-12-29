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
	     WavConverter::ptr _src;
	  };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_DVIADPCMDECODERCONVERTER_HPP