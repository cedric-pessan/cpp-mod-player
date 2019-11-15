#ifndef MODS_WAV_TRUSPEECHDECODERCONVERTER_HPP
#define MODS_WAV_TRUSPEECHDECODERCONVERTER_HPP

#include "mods/utils/BitReader.hpp"
#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class TruspeechDecoderConverter : public WavConverter
          {
           public:
             explicit TruspeechDecoderConverter(WavConverter::ptr src);
             
             TruspeechDecoderConverter() = delete;
             TruspeechDecoderConverter(const TruspeechDecoderConverter&) = delete;
             TruspeechDecoderConverter(TruspeechDecoderConverter&&) = delete;
             TruspeechDecoderConverter& operator=(const TruspeechDecoderConverter&) = delete;
             TruspeechDecoderConverter& operator=(TruspeechDecoderConverter&&) = delete;
             ~TruspeechDecoderConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             mods::utils::RWBuffer<u8> allocateNewTempBuffer(size_t len);
             
             template<typename ARRAY>
               static mods::utils::RWBuffer<typename ARRAY::value_type> initializeArrayRWBuffer(ARRAY& backArray);
             
             void decodeTruspeechFrame();
             
             WavConverter::ptr _src;
             
             std::vector<u8> _tempVec;
             mods::utils::RWBuffer<u8> _destinationBuffer;
             mods::utils::RWBuffer<u8> _decodedBuffer;
             mods::utils::RWBuffer<u8>::const_iterator _itDecodedBuffer;
             
             constexpr static int TRUSPEECH_FRAME_HEADER_SIZE = 1;
             constexpr static int LOW_BITRATE_FRAME_SIZE = 20;
             constexpr static int HIGH_BITRATE_FRAME_SIZE = 24;
             
             std::array<u8, HIGH_BITRATE_FRAME_SIZE> _encodedArray;
             mods::utils::RWBuffer<u8> _encodedBuffer;
             
             mods::utils::BitReader _bitReader;
             
             enum struct BitRate
               {
                  Low, High
               };
             enum struct VoiceActive
               {
                  No, Yes
               };
             
             BitRate _bitRate;
             VoiceActive _vad;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_TRUSPEECHDECODERCONVERTER_HPP
