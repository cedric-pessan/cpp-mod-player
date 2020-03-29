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
             void filtersMerge();
             void applyTwoPointFilter(int subframe);
             void placePulses(int subframe);
             void updateFilters(int subframe);
             void synth(int subframe);
             
             WavConverter::ptr _src;
             
             constexpr static int TRUSPEECH_DECODED_FRAME_SIZE = 240 * 2;
             
             std::array<u8, TRUSPEECH_DECODED_FRAME_SIZE> _decodedArray;
             mods::utils::RWBuffer<u8> _decodedBuffer;
             mods::utils::RWBuffer<u8>::const_iterator _itDecodedBuffer;
             std::array<mods::utils::RWBuffer<s16>, 4> _subframes;
             
             constexpr static int TRUSPEECH_ENCODED_FRAME_SIZE = 32;
             
             std::array<u8, TRUSPEECH_ENCODED_FRAME_SIZE> _encodedArray;
             mods::utils::RWBuffer<u8> _encodedBuffer;
             
             mods::utils::BitReader<mods::utils::ByteSwap::U32, mods::utils::BitOrder::MsbToLsb> _bitReader;
             
             // parameters
             std::array<s16,8> _vector;
             bool _flag = false;
             std::array<s32,2> _offset1;
             std::array<s32,4> _offset2;
             std::array<s32,4> _pulseval;
             std::array<s32,4> _pulsepos;
             std::array<s32,4> _pulseoff;
             
             // tmps
             using CorrelatedVector = std::array<s16,8>;
             std::array<CorrelatedVector,2> _correlatedVectors;
             int _currentCorrelatedVector = 0;
             s32 _filtVal = 0;
             
             std::array<s16, 32> _filters;
             std::array<s32, 146> _filterBuffer;
             std::array<s16, 60> _newVector;
             std::array<s16, 8> _tmp1;
             std::array<s16, 8> _tmp2;
             std::array<s16, 8> _tmp3;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_TRUSPEECHDECODERCONVERTER_HPP
