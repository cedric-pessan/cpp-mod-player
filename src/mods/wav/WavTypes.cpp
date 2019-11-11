
#include "mods/wav/WavTypes.hpp"

namespace mods
{
   namespace wav
     {
        const std::string& toString(WavAudioFormat fmt)         
          {
             static const std::string PCM = "PCM";
             static const std::string IEEE_FLOAT = "IEEE FLOAT";
             static const std::string A_LAW = "A-LAW";
             static const std::string MU_LAW = "MU_LAW";
             static const std::string TRUSPEECH = "TRUSPEECH";
             static const std::string GSM = "GSM";
             static const std::string EXTENSIBLE = "EXTENSIBLE";
             switch(fmt)
               {
                case WavAudioFormat::PCM:
                  return PCM;
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
                case WavAudioFormat::EXTENSIBLE:
                  return EXTENSIBLE;
               }
             return PCM; // doesn't matter, it can't be reached
          }
     } // namespace wav
} // namespace mods

