#ifndef MODS_WAV_GSMDECODERCONVERTER_HPP
#define MODS_WAV_GSMDECODERCONVERTER_HPP

#include "mods/wav/GSMBitReader.hpp"
#include "mods/wav/GSMInt16.hpp"
#include "mods/wav/WavConverter.hpp"

namespace mods
{
   namespace wav
     {
        class GSMDecoderConverter : public WavConverter
          {
           public:
             explicit GSMDecoderConverter(WavConverter::ptr src);
             
             GSMDecoderConverter() = delete;
             GSMDecoderConverter(const GSMDecoderConverter&) = delete;
             GSMDecoderConverter(GSMDecoderConverter&&) = delete;
             GSMDecoderConverter& operator=(const GSMDecoderConverter&) = delete;
             GSMDecoderConverter& operator=(GSMDecoderConverter&&) = delete;
             ~GSMDecoderConverter() override = default;
             
             bool isFinished() const override;
             void read(mods::utils::RWBuffer<u8>* buf, int len) override;
             
           private:
             template<typename ARRAY>
               static mods::utils::RWBuffer<typename ARRAY::value_type> initializeArrayBuffer(const ARRAY& backArray);
             
             void decodeGSMFrame();
             void uncompressGSMFrame();
             void readParameters();
             
             void inverseAPCM(int subframe);
             void RPEGridPosition(int subframe);
             void longTermPrediction(int subframe);
             void shortTermSynthesis();
             void LARDecode();
             void LARInterpolation_0_12();
             void LARInterpolation_13_26();
             void LARInterpolation_27_39();
             void LARInterpolation_40_159();
             void computeReflectionCoefficients();
             void shortTermSynthesisFiltering(int k_start, int k_end);
             void deEmphasisFiltering();
             void upscale();
             
             WavConverter::ptr _src;
             
             constexpr static int GSM_DECODED_FRAME_SAMPLES = 160;
             constexpr static int GSM_DECODED_FRAME_SIZE = GSM_DECODED_FRAME_SAMPLES * 2;
             
             constexpr static int GSM_ENCODED_FRAME_SIZE = 260;
             constexpr static int GSM_ENCODED_PACK_SIZE = (GSM_ENCODED_FRAME_SIZE * 2) / 8;
             
             std::array<u8, GSM_DECODED_FRAME_SIZE> _decodedArray;
             mods::utils::RWBuffer<u8> _decodedBuffer;
             mods::utils::RWBuffer<u8>::const_iterator _itDecodedBuffer;
             
             std::array<u8, GSM_ENCODED_PACK_SIZE> _encodedArray;
             mods::utils::RWBuffer<u8> _encodedBuffer;
             
             bool _evenFrame = true;
             GSMBitReader _bitReader;
             
             std::array<GSMInt16, 8> _LARc_array;
             mods::utils::RWBuffer<GSMInt16> _LARc;
             
             using LARppArray = std::array<GSMInt16, 8>;
             using LARppBuffer = mods::utils::RWBuffer<GSMInt16>;
             std::array<LARppArray, 2> _LARppArrays;
             std::array<LARppBuffer, 2> _LARppBufferArray;
             mods::utils::RWBuffer<LARppBuffer> _LARpp;
             int _currentLARpp = 0;
             
             std::array<GSMInt16, 8> _LARpArray;
             mods::utils::RWBuffer<GSMInt16> _LARp;
             
             std::array<GSMInt16, 8> _rpArray;
             mods::utils::RWBuffer<GSMInt16> _rp;
             
             std::array<GSMInt16, 9> _vArray;
             mods::utils::RWBuffer<GSMInt16> _v;
             
             std::array<GSMInt16, GSM_DECODED_FRAME_SAMPLES> _srArray;
             mods::utils::RWBuffer<GSMInt16> _sr;
             
             GSMInt16 _msr;
             std::array<GSMInt16, GSM_DECODED_FRAME_SAMPLES> _sroArray;
             mods::utils::RWBuffer<GSMInt16> _sro;
             
             constexpr static std::array<int, 8> _LAR_SIZES_array
               {
                  6, 6, 5, 5, 4, 4, 3, 3
               };
             mods::utils::RBuffer<int> _LAR_SIZES;
             
             constexpr static std::array<GSMInt16, 8> _FAC_array
               {
                  18431, 20479, 22527, 24575, 26623, 28671, 30719, 32767
               };
             mods::utils::RBuffer<GSMInt16> _FAC;
             
             constexpr static std::array<GSMInt16, 4> _QLB_array
               {
                  3277, 11469, 21299, 32767
               };
             mods::utils::RBuffer<GSMInt16> _QLB;
             
             constexpr static std::array<GSMInt16, 8> _MIC_array
               {
                  -32, -32, -16, -16, -8, -8, -4, -4
               };
             mods::utils::RBuffer<GSMInt16> _MIC;
             
             constexpr static std::array<GSMInt16, 8> _B_array
               {
                  0, 0, 2048, -2560, 94, -1792, -341, -1144
               };
             mods::utils::RBuffer<GSMInt16> _B;
             
             constexpr static std::array<GSMInt16, 8> _INVA_array
               {
                  13107, 13107, 13107, 13107, 19223, 17476, 31454, 29708
               };
             mods::utils::RBuffer<GSMInt16> _INVA;
             
             struct SubFrame
               {
                private:
                  std::array<GSMInt16, 13> _xArray;
                  std::array<GSMInt16, 13> _xpArray;
                  std::array<GSMInt16, 40> _epArray;
                  
                public:
                  SubFrame();
                  
                  // LTP
                  GSMInt16 N;
                  GSMInt16 b;
                  
                  // RPE
                  GSMInt16 M;
                  GSMInt16 Xmax;
                  mods::utils::RWBuffer<GSMInt16> x;
                  
                  // temporaries
                  mods::utils::RWBuffer<GSMInt16> xp;
                  mods::utils::RWBuffer<GSMInt16> ep;
               };
             
             std::array<SubFrame, 4> _subframesArray;
             mods::utils::RWBuffer<SubFrame> _subframes;
             
             GSMInt16 _nrp = 40;
             
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
                  RingArray& operator=(const RingArray&) = delete;
                  RingArray& operator=(RingArray&&) = delete;
                  
                  GSMInt16& operator[](const GSMInt16& index);
                  
                  void slideOrigin(int offset);
                  
                private:
                  constexpr static int _ARRAY_SIZE = 160;
                  
                  int _origin = 0;
                  std::array<GSMInt16, _ARRAY_SIZE> _array;
               };
             
             RingArray _drp;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_GSMDECODERCONVERTER_HPP
