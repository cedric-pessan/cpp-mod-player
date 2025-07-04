#ifndef MODS_WAV_GSMDECODERCONVERTER_HPP
#define MODS_WAV_GSMDECODERCONVERTER_HPP

#include "mods/converters/Converter.hpp"
#include "mods/utils/BitReader.hpp"
#include "mods/wav/GSMInt16.hpp"

#include <array>
#include <functional>

namespace mods
{
   namespace wav
     {
        class GSMDecoderConverter : public mods::converters::Converter<s16>
          {
           public:
             explicit GSMDecoderConverter(Converter<u8>::ptr src);
             
             GSMDecoderConverter() = delete;
             GSMDecoderConverter(const GSMDecoderConverter&) = delete;
             GSMDecoderConverter(GSMDecoderConverter&&) = delete;
             auto operator=(const GSMDecoderConverter&) -> GSMDecoderConverter& = delete;
             auto operator=(GSMDecoderConverter&&) -> GSMDecoderConverter& = delete;
             ~GSMDecoderConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<s16>* buf) override;
             
             static constexpr auto isValidAsBitsPerSample(int bitsPerSample) -> bool
               {
                  return (bitsPerSample % GSM_ENCODED_FRAME_SIZE) == 0;
               }
             
             static constexpr auto getOutputBitsPerSample() -> int
               {
                  constexpr int outputBitsPerSample = 13;
                  return outputBitsPerSample;
               }
             
             static auto getBitsPerSampleRequirementsString() -> std::string&;
             
           private:
             template<typename ARRAY>
               static auto initializeArrayRWBuffer(ARRAY& backArray) -> mods::utils::RWBuffer<typename ARRAY::value_type>;
             
             enum struct ShortTermSynthesisFilteringRanges : u8
               {
                  range_0_12,
                    range_13_26,
                    range_27_39,
                    range_40_159
               };
             
             struct ShortTermSynthesisFilteringParameters
               {
                  ShortTermSynthesisFilteringRanges kRange;
                  int startk;
                  int endk;
               };
             
             static const std::array<ShortTermSynthesisFilteringParameters, 4> shortTermSynthesisParameters;
             
             void decodeGSMFrame();
             void uncompressGSMFrame();
             void readParameters();
             
             void inverseAPCM(int subframe);
             void RPEGridPosition(int subframe);
             void longTermPrediction(int subframe);
             void shortTermSynthesis();
             void larDecode();
             void larInterpolation_0_12();
             void larInterpolation_13_26();
             void larInterpolation_27_39();
             void larInterpolation_40_159();
             void computeReflectionCoefficients();
             void shortTermSynthesisFiltering(const ShortTermSynthesisFilteringParameters& params);
             void deEmphasisFiltering();
             void upscale();
             
             Converter<u8>::ptr _src;
             
             constexpr static int GSM_DECODED_FRAME_SIZE = 160;
             
             constexpr static int GSM_ENCODED_FRAME_SIZE = 260;
             constexpr static int GSM_ENCODED_PACK_SIZE = (GSM_ENCODED_FRAME_SIZE * 2) / 8;
             
             std::array<s16, GSM_DECODED_FRAME_SIZE> _decodedArray;
             mods::utils::RWBuffer<s16> _decodedBuffer;
             mods::utils::RWBuffer<s16>::const_iterator _itDecodedBuffer;
             
             std::array<u8, GSM_ENCODED_PACK_SIZE> _encodedArray;
             mods::utils::RWBuffer<u8> _encodedBuffer;
             
             bool _evenFrame = true;
             mods::utils::BitReader<mods::utils::ByteSwap::None, mods::utils::BitOrder::LsbToMsb> _bitReader;
             
             constexpr static int _numberOfLarCoefficients = 8;
             
             std::array<GSMInt16, _numberOfLarCoefficients> _larC;
             
             using larPPArray = std::array<GSMInt16, _numberOfLarCoefficients>;
             std::array<larPPArray, 2> _larPP;
             int _currentLarPP = 0;
             
             std::array<GSMInt16, _numberOfLarCoefficients> _larP;
             
             std::array<GSMInt16, _numberOfLarCoefficients> _rp;
             
             std::array<GSMInt16, _numberOfLarCoefficients+1> _v;
             
             std::array<GSMInt16, GSM_DECODED_FRAME_SIZE> _sr;
             
             GSMInt16 _msr;
             std::array<GSMInt16, GSM_DECODED_FRAME_SIZE> _sro;
             
             constexpr static std::array<int, 8> _larSizes
               {
                  6, 6, 5, 5, 4, 4, 3, 3
               };
             
             constexpr static std::array<GSMInt16, 8> _fac
               {
                  18431, 20479, 22527, 24575, 26623, 28671, 30719, 32767
               };
             
             constexpr static std::array<GSMInt16, 4> _qlb
               {
                  3277, 11469, 21299, 32767
               };
             
             constexpr static std::array<GSMInt16, 8> _mic
               {
                  -32, -32, -16, -16, -8, -8, -4, -4
               };
             
             constexpr static std::array<GSMInt16, 8> _bConsts
               {
                  0, 0, 2048, -2560, 94, -1792, -341, -1144
               };
             
             constexpr static std::array<GSMInt16, 8> _inva
               {
                  13107, 13107, 13107, 13107, 19223, 17476, 31454, 29708
               };
             
             constexpr static int _numberOfRPEPulses = 13;
             constexpr static int _rpeBlockSize = 40;
             
             struct SubFrame
               {
                  // LTP
                  GSMInt16 N;
                  GSMInt16 b;
                  
                  // RPE
                  GSMInt16 M;
                  GSMInt16 Xmax;
                  std::array<GSMInt16, _numberOfRPEPulses> x;
                  
                  // temporaries
                  std::array<GSMInt16, _numberOfRPEPulses> xp;
                  std::array<GSMInt16, _rpeBlockSize> ep;
               };
             
             std::array<SubFrame, 4> _subframes;
             
             constexpr static int _initialNRP = 40;
             
             GSMInt16 _nrp = _initialNRP;
             
             constexpr static int LTP_LAG_SIZE = 7;
             constexpr static int LTP_GAIN_SIZE = 2;
             
             constexpr static int RPE_GRID_POSITION_SIZE = 2;
             constexpr static int RPE_BLOCK_AMPLITUDE_SIZE = 6;
             constexpr static int RPE_PULSE_SIZE = 3;
             
             class RingArray
               {
                public:
                  RingArray() = default;
                  ~RingArray() = default;
                  
                  RingArray(const RingArray&) = delete;
                  RingArray(RingArray&&) = delete;
                  auto operator=(const RingArray&) -> RingArray& = delete;
                  auto operator=(RingArray&&) -> RingArray& = delete;
                  
                  auto operator[](const GSMInt16& index) -> GSMInt16&;
                  
                  void slideOrigin(int offset);
                  
                private:
                  constexpr static int _ringArraySize = 160;
                  
                  int _origin = 0;
                  std::array<GSMInt16, _ringArraySize> _array;
               };
             
             RingArray _drp;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_GSMDECODERCONVERTER_HPP
