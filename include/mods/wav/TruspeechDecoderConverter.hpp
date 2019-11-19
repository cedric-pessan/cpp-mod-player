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
             
             int _lpc;
             int _acl0;
             int _acl1;
             int _acl2;
             int _acl3;
             int _gain0;
             int _gain1;
             int _gain2;
             int _gain3;
             int _grid0;
             int _grid1;
             int _grid2;
             int _grid3;
             int _pos0;
             int _pos1;
             int _pos2;
             int _pos3;
             int _psig0;
             int _psig1;
             int _psig2;
             int _psig3;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_TRUSPEECHDECODERCONVERTER_HPP
