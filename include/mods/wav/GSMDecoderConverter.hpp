#ifndef MODS_WAV_GSMDECODERCONVERTER_HPP
#define MODS_WAV_GSMDECODERCONVERTER_HPP

#include "mods/utils/BitReader.hpp"
#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class GSMDecoderConverter : public WavConverter
          {
           public:
             GSMDecoderConverter(WavConverter::ptr src);
             
             GSMDecoderConverter() = delete;
             GSMDecoderConverter(const GSMDecoderConverter&) = delete;
             GSMDecoderConverter(GSMDecoderConverter&&) = delete;
             GSMDecoderConverter& operator=(const GSMDecoderConverter&) = delete;
             GSMDecoderConverter& operator=(GSMDecoderConverter&&) = delete;
             ~GSMDecoderConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             mods::utils::RWBuffer<u8> allocateNewBuffer(std::vector<u8>& backVec, size_t len);
             
             void decodeGSMFrame();
             void uncompressGSMFrame();
             void readParameters();
             
             WavConverter::ptr _src;
             
             std::vector<u8> _decodedVec;
             mods::utils::RWBuffer<u8> _decodedBuffer;
             mods::utils::RWBuffer<u8>::const_iterator _itDecodedBuffer;
             
             std::vector<u8> _encodedVec;
             mods::utils::RWBuffer<u8> _encodedBuffer;
             
             bool _evenFrame = true;
             mods::utils::BitReader _bitReader;
             
             constexpr static int GSM_DECODED_FRAME_SAMPLES = 160;
             constexpr static int GSM_DECODED_FRAME_SIZE = GSM_DECODED_FRAME_SAMPLES * 2;
             
             constexpr static int GSM_ENCODED_FRAME_SIZE = 260;
             constexpr static int GSM_ENCODED_PACK_SIZE = (GSM_ENCODED_FRAME_SIZE * 2) / 8;
             
             std::array<int, 8> _LAR;
             constexpr static std::array<int, 8> _LAR_SIZES
               {
                  6, 6, 5, 5, 4, 4, 3, 3
               };
             
             struct SubFrame
               {
                  // LTP
                  int N;
                  int b;
                  
                  // RPE
                  int M;
                  int Xmax;
                  std::array<int, 13> x;
               };
             
             std::array<SubFrame, 4> _subframes;
             
             constexpr static int LTP_LAG_SIZE = 7;
             constexpr static int LTP_GAIN_SIZE = 2;
             
             constexpr static int RPE_GRID_POSITION_SIZE = 2;
             constexpr static int RPE_BLOCK_AMPLITUDE_SIZE = 6;
             constexpr static int RPE_PULSE_SIZE = 3;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_GSMDECODERCONVERTER_HPP