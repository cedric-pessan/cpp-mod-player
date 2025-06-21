
#include "mods/wav/WavTypes.hpp"

#include <string>

namespace mods
{
   namespace wav
     {
        auto toString(WavAudioFormat fmt) -> const std::string&
          {
             static const std::string PCM = "PCM";
             static const std::string ADPCM = "ADPCM";
             static const std::string IEEE_FLOAT = "IEEE FLOAT";
             static const std::string A_LAW = "A-LAW";
             static const std::string MU_LAW = "MU_LAW";
             static const std::string TRUSPEECH = "TRUSPEECH";
             static const std::string GSM = "GSM";
             static const std::string OKI_ADPCM = "OKI/ADPCM";
	     static const std::string DVI_ADPCM = "DVI/ADPCM";
             static const std::string EXTENSIBLE = "EXTENSIBLE";
             switch(fmt)
               {
                case WavAudioFormat::PCM:
                  return PCM;
                case WavAudioFormat::ADPCM:
                  return ADPCM;
                case WavAudioFormat::IEEE_FLOAT:
                  return IEEE_FLOAT;
                case WavAudioFormat::A_LAW:
                  return A_LAW;
                case WavAudioFormat::MU_LAW:
                  return MU_LAW;
                case WavAudioFormat::TRUSPEECH:
                  return TRUSPEECH;
                case WavAudioFormat::GSM:
                  return GSM;
                case WavAudioFormat::OKI_ADPCM:
                  return OKI_ADPCM;
		case WavAudioFormat::DVI_ADPCM:
		  return DVI_ADPCM;
                case WavAudioFormat::EXTENSIBLE:
                  return EXTENSIBLE;
               }
             return PCM; // doesn't matter, it can't be reached
          }
     } // namespace wav
} // namespace mods

