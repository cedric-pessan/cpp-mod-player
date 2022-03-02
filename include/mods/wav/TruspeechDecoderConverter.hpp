#ifndef MODS_WAV_TRUSPEECHDECODERCONVERTER_HPP
#define MODS_WAV_TRUSPEECHDECODERCONVERTER_HPP

#include "mods/converters/Converter.hpp"
#include "mods/utils/BitReader.hpp"

namespace mods
{
   namespace wav
     {
        class TruspeechDecoderConverter : public mods::converters::Converter
          {
           public:
             explicit TruspeechDecoderConverter(Converter::ptr src);
             
             TruspeechDecoderConverter() = delete;
             TruspeechDecoderConverter(const TruspeechDecoderConverter&) = delete;
             TruspeechDecoderConverter(TruspeechDecoderConverter&&) = delete;
             auto operator=(const TruspeechDecoderConverter&) -> TruspeechDecoderConverter& = delete;
             auto operator=(TruspeechDecoderConverter&&) -> TruspeechDecoderConverter& = delete;
             ~TruspeechDecoderConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
             static constexpr auto getOutputBitsPerSample() -> int
               {
                  constexpr int outputBitsPerSample = 16;
                  return outputBitsPerSample;
               }
             
           private:
             
             template<typename ARRAY>
               static auto initializeArrayRWBuffer(ARRAY& backArray) -> mods::utils::RWBuffer<typename ARRAY::value_type>;
             
             void decodeTruspeechFrame();
             void readParameters();
             void correlateFilter();
             void filtersMerge();
             void applyTwoPointFilter(int subframe);
             void placePulses(int subframe);
             void updateFilters(int subframe);
             void synth(int subframe);
             
             Converter::ptr _src;
             
             constexpr static int TRUSPEECH_DECODED_FRAME_SIZE = 240 * 2;
             
             std::array<u8, TRUSPEECH_DECODED_FRAME_SIZE> _decodedArray;
             mods::utils::RWBuffer<u8> _decodedBuffer;
             mods::utils::RWBuffer<u8>::const_iterator _itDecodedBuffer;
             std::array<mods::utils::RWBuffer<s16>, 4> _subframes;
             
             constexpr static int TRUSPEECH_ENCODED_FRAME_SIZE = 32;
             
             std::array<u8, TRUSPEECH_ENCODED_FRAME_SIZE> _encodedArray;
             mods::utils::RWBuffer<u8> _encodedBuffer;
             
             mods::utils::BitReader<mods::utils::ByteSwap::U32, mods::utils::BitOrder::MsbToLsb> _bitReader;
             
             static constexpr int _numberOfFilterCoefficients = 8;
             static constexpr int _filterBufferLength = 146;
             static constexpr int _subframeLength = 60;
             
             // parameters
             std::array<s16,_numberOfFilterCoefficients> _vector;
             bool _flag = false;
             std::array<s32,2> _offset1;
             std::array<s32,4> _offset2;
             std::array<s32,4> _pulseval;
             std::array<s32,4> _pulsepos;
             std::array<s32,4> _pulseoff;
             
             // tmps
             using CorrelatedVector = std::array<s16,_numberOfFilterCoefficients>;
             std::array<CorrelatedVector,2> _correlatedVectors;
             int _currentCorrelatedVector = 0;
             s32 _filtVal = 0;
             
             using Filter = std::array<s16, _numberOfFilterCoefficients>;
             std::array<Filter, 4> _filters;
             std::array<s32, _filterBufferLength> _filterBuffer;
             std::array<s16, _subframeLength> _newVector;
             std::array<s16, _numberOfFilterCoefficients> _tmp1;
             std::array<s16, _numberOfFilterCoefficients> _tmp2;
             std::array<s16, _numberOfFilterCoefficients> _tmp3;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_TRUSPEECHDECODERCONVERTER_HPP
