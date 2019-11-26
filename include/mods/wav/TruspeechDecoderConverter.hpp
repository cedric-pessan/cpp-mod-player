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
             
             template<typename ARRAY>
               static mods::utils::RWBuffer<typename ARRAY::value_type> initializeArrayRWBuffer(ARRAY& backArray);
             
             void decodeTruspeechFrame();
             void readParameters();
             void correlateFilter();
             
             WavConverter::ptr _src;
             
             constexpr static int TRUSPEECH_DECODED_FRAME_SIZE = 240;
             
             std::vector<u8> _tempVec;
             std::array<u8, TRUSPEECH_DECODED_FRAME_SIZE> _decodedArray;
             mods::utils::RWBuffer<u8> _decodedBuffer;
             mods::utils::RWBuffer<u8>::const_iterator _itDecodedBuffer;
             
             constexpr static int TRUSPEECH_ENCODED_FRAME_SIZE = 32;
             
             std::array<u8, TRUSPEECH_ENCODED_FRAME_SIZE> _encodedArray;
             mods::utils::RWBuffer<u8> _encodedBuffer;
             
             mods::utils::BitReader _bitReader;
             
             // parameters
             s16 _vector[8];
             bool _flag;
             s32 _offset1[2];
             s32 _offset2[4];
             s32 _pulseval[4];
             s32 _pulsepos[4];
             s32 _pulseoff[4];
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_TRUSPEECHDECODERCONVERTER_HPP
