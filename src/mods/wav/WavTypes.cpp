
#include "mods/wav/WavTypes.hpp"

namespace mods
{
   namespace wav
     {
        const std::string& toString(WavAudioFormat fmt)         
          {
             static const std::string PCM = "PCM";
             static const std::string A_LAW = "A-LAW";
             static const std::string EXTENSIBLE = "EXTENSIBLE";
             switch(fmt)
               {
                case WavAudioFormat::PCM:
                  return PCM;
                case WavAudioFormat::A_LAW:
                  return A_LAW;
                case WavAudioFormat::EXTENSIBLE:
                  return EXTENSIBLE;
               }
             return PCM; // doesn't matter, it can't be reached
          }
     } // namespace wav
} // namespace mods

