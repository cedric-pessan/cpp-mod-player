
#include "mods/wav/GSMDecoderConverter.hpp"

namespace mods
{
   namespace wav
     {
        constexpr std::array<int, 8> GSMDecoderConverter::_LAR_SIZES;
        constexpr std::array<GSMInt16, 8> GSMDecoderConverter::_FAC;
        constexpr std::array<GSMInt16, 4> GSMDecoderConverter::_QLB;
        constexpr std::array<GSMInt16, 8> GSMDecoderConverter::_MIC;
        constexpr std::array<GSMInt16, 8> GSMDecoderConverter::_B;
        constexpr std::array<GSMInt16, 8> GSMDecoderConverter::_INVA;
        
        GSMDecoderConverter::GSMDecoderConverter(WavConverter::ptr src)
          : _src(std::move(src)),
          _decodedArray {},
          _decodedBuffer(initializeArrayRWBuffer(_decodedArray)),
          _itDecodedBuffer(_decodedBuffer.RBuffer<u8>::end()),
          _encodedArray {},
          _encodedBuffer(initializeArrayRWBuffer(_encodedArray)),
          _bitReader(_encodedBuffer)
            {
            }
        
        auto GSMDecoderConverter::isFinished() const -> bool
          {
             return _itDecodedBuffer == _decodedBuffer.RBuffer<u8>::end() && _src->isFinished();
          }
        
        void GSMDecoderConverter::read(mods::utils::RWBuffer<u8>* buf, size_t len)
          {
             size_t count = 0;
             while(count < len)
               {
                  if(_itDecodedBuffer != _decodedBuffer.RBuffer<u8>::end())
                    {
                       (*buf)[count++] = *_itDecodedBuffer;
                       ++_itDecodedBuffer;
                    }
                  else
                    {
                       if(_src->isFinished())
                         {
                            (*buf)[count++] = 0;
                         }
                       else
                         {
                            decodeGSMFrame();
                            _itDecodedBuffer = _decodedBuffer.RBuffer<u8>::begin();
                         }
                    }
               }
          }
        
        template<typename ARRAY>
          auto GSMDecoderConverter::initializeArrayRWBuffer(ARRAY& backArray) -> mods::utils::RWBuffer<typename ARRAY::value_type>
            {
               auto* ptr = static_cast<u8*>(static_cast<void*>(backArray.data()));
               auto len = backArray.size();
               auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
               auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, len * sizeof(typename ARRAY::value_type), std::move(deleter));
               return mods::utils::RWBuffer<u8>(buffer).slice<typename ARRAY::value_type>(0, len);
            }
        
        void GSMDecoderConverter::decodeGSMFrame()
          {
             if(_evenFrame)
               {
                  _src->read(&_encodedBuffer, GSM_ENCODED_PACK_SIZE);
                  _bitReader.reset();
               }
             
             uncompressGSMFrame();
             _evenFrame = !_evenFrame;
          }
        
        void GSMDecoderConverter::uncompressGSMFrame()
          {
             readParameters();
             
             for(int i=0; i<4; ++i)
               {
                  inverseAPCM(i);
                  RPEGridPosition(i);
                  longTermPrediction(i);
               }
             shortTermSynthesis();
             deEmphasisFiltering();
             upscale();
          }
        
        void GSMDecoderConverter::readParameters()
          {
             using mods::utils::at;
             for(size_t i=0; i<_LARc.size(); ++i)
               {
                  at(_LARc,i) = _bitReader.read(at(_LAR_SIZES,i));
               }
             
             for(int i=0; i<4; ++i)
               {
                  auto& subframe = at(_subframes,i);
                  subframe.N = _bitReader.read(LTP_LAG_SIZE);
                  subframe.b = _bitReader.read(LTP_GAIN_SIZE);
                  
                  subframe.M = _bitReader.read(RPE_GRID_POSITION_SIZE);
                  subframe.Xmax = _bitReader.read(RPE_BLOCK_AMPLITUDE_SIZE);
                  for(size_t j=0; j<subframe.x.size(); ++j)
                    {
                       at(subframe.x,j) = _bitReader.read(RPE_PULSE_SIZE);
                    }
               }
          }
        
        void GSMDecoderConverter::inverseAPCM(int subframe)
          {
             using mods::utils::at;
             
             GSMInt16 exponent = 0;
             auto xmax = at(_subframes,subframe).Xmax;
             static constexpr int xmaxWithExponentTestValue = 15;
             if(xmax > xmaxWithExponentTestValue)
               {
                  exponent = (xmax >> 3) - 1;
               }
             GSMInt16 mantissa = xmax - (exponent << 3);
             
             if(mantissa == 0)
               {
                  static constexpr int denormalizedMantissa = 15;
                  exponent = -4;
                  mantissa = denormalizedMantissa;
               }
             else
               {
                  bool itest = false;
                  for(int i=0; i<3; ++i)
                    {
                       static constexpr int minNormalizedMantissa = 7;
                       if(mantissa > minNormalizedMantissa)
                         {
                            itest = true;
                         }
                       if(!itest)
                         {
                            mantissa = (mantissa << 1) + 1;
                            --exponent;
                         }
                    }
               }
             static constexpr int mantissaSignOffset = 8;
             mantissa -= mantissaSignOffset;
             
             static constexpr int exponentOffset = 6;
             
             GSMInt16 temp1 = at(_FAC, mantissa.getValue());
             GSMInt16 temp2 = exponentOffset - exponent;
             GSMInt16 temp3 = 1 << (temp2 - 1);
             
             auto& x = at(_subframes, subframe).x;
             auto& xp = at(_subframes, subframe).xp;
             
             static constexpr int pulseDecodeOffset = -7;
             static constexpr int pulseFixedPointConversionShift = 12;
             
             for(size_t i=0; i <xp.size(); ++i)
               {
                  GSMInt16 temp = (at(x,i) << 1) + pulseDecodeOffset;;
                  temp <<= pulseFixedPointConversionShift;
                  temp = temp.mult_round(temp1);
                  temp += temp3;
                  at(xp,i) = temp >> temp2;
               }
          }
        
        void GSMDecoderConverter::RPEGridPosition(int subframe)
          {
             using mods::utils::at;
             
             auto& sub = at(_subframes,subframe);
             auto& ep = sub.ep;
             auto& M = sub.M;
             auto& xp = sub.xp;
             
             for(size_t k=0; k<ep.size(); ++k)
               {
                  at(ep,k) = 0;
               }
             for(size_t i=0; i<xp.size(); ++i)
               {
                  at(ep,M.getValue() + (3*i)) = at(xp,i);
               }
          }
        
        void GSMDecoderConverter::longTermPrediction(int subframe)
          {
             using mods::utils::at;
             
             auto& sub = at(_subframes,subframe);
             auto& Ncr = sub.N;
             auto& bcr = sub.b;
             auto& erp = sub.ep;
             
             static constexpr int minNr = 40;
             static constexpr int maxNr = 120;
             
             auto Nr = Ncr;
             if(Ncr < minNr)
               {
                  Nr = _nrp;
               }
             if(Ncr > maxNr)
               {
                  Nr = _nrp;
               }
             _nrp = Nr;
             
             auto& brp = at(_QLB,bcr.getValue());
             
             for(size_t k=0; k<erp.size(); ++k)
               {
                  auto drpp = brp.mult_round(_drp[k - Nr]);
                  _drp[k] = at(erp,k) + drpp;
               }
             
             _drp.slideOrigin(_RPEBlockSize);
          }
        
        namespace
          {
             enum struct ShortTermSynthesisFilteringRanges
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
             
             const std::array<ShortTermSynthesisFilteringParameters, 4> shortTermSynthesisParameters
               {
                    {
                         { ShortTermSynthesisFilteringRanges::range_0_12, 0, 12 },
                         { ShortTermSynthesisFilteringRanges::range_13_26, 13, 26 },
                         { ShortTermSynthesisFilteringRanges::range_27_39, 27, 39 },
                         { ShortTermSynthesisFilteringRanges::range_40_159, 40, 159 }
                    }
               };
          } // namespace
        
        void GSMDecoderConverter::shortTermSynthesis()
          {
             LARDecode();
             
             for(auto& param : shortTermSynthesisParameters)
               {
                  switch(param.kRange)
                    {
                     case ShortTermSynthesisFilteringRanges::range_0_12:
                       LARInterpolation_0_12();
                       break;
                     case ShortTermSynthesisFilteringRanges::range_13_26:
                       LARInterpolation_13_26();
                       break;
                     case ShortTermSynthesisFilteringRanges::range_27_39:
                       LARInterpolation_27_39();
                       break;
                     case ShortTermSynthesisFilteringRanges::range_40_159:
                       LARInterpolation_40_159();
                       break;
                    }
                  computeReflectionCoefficients();
                  shortTermSynthesisFiltering(param.startk, param.endk);
               }
          }
        
        void GSMDecoderConverter::LARDecode()
          {
             using mods::utils::at;
             
             _currentLARpp = 1 - _currentLARpp;
             auto& LARpp = at(_LARpp, _currentLARpp);
             
             static constexpr int scaleShift = 10;
             
             for(size_t i=0; i<LARpp.size(); ++i)
               {
                  GSMInt16 temp1 = (at(_LARc,i) + at(_MIC,i)) << scaleShift;
                  GSMInt16 temp2 = at(_B,i) << 1;
                  temp1 -= temp2;
                  temp1 = at(_INVA,i).mult_round(temp1);
                  at(LARpp,i) = temp1 + temp1;
               }
          }
        
        void GSMDecoderConverter::LARInterpolation_0_12()
          {
             using mods::utils::at;
             auto& LARppOld = at(_LARpp,1-_currentLARpp);
             auto& LARpp = at(_LARpp,_currentLARpp);
             
             for(int i=0; i<_numberOfLARCoefficients; ++i)
               {
                  at(_LARp,i) = (at(LARppOld,i) >> 2) + (at(LARpp,i) >> 2);
                  at(_LARp,i) += (at(LARppOld,i) >> 1);
               }
          }
        
        void GSMDecoderConverter::LARInterpolation_13_26()
          {
             using mods::utils::at;
             auto& LARppOld = at(_LARpp,1-_currentLARpp);
             auto& LARpp = at(_LARpp,_currentLARpp);
             
             for(int i=0; i<_numberOfLARCoefficients; ++i)
               {
                  at(_LARp,i) = (at(LARppOld,i) >> 1) + (at(LARpp,i) >> 1);
               }
          }
        
        void GSMDecoderConverter::LARInterpolation_27_39()
          {
             using mods::utils::at;
             auto& LARppOld = at(_LARpp,1-_currentLARpp);
             auto& LARpp = at(_LARpp,_currentLARpp);
             
             for(int i=0; i<_numberOfLARCoefficients; ++i)
               {
                  at(_LARp,i) = (at(LARppOld,i) >> 2) + (at(LARpp,i) >> 2);
                  at(_LARp,i) += (at(LARpp,i) >> 1);
               }
          }
        
        void GSMDecoderConverter::LARInterpolation_40_159()
          {
             using mods::utils::at;
             auto& LARpp = at(_LARpp,_currentLARpp);
             
             for(int i=0; i<_numberOfLARCoefficients; ++i)
               {
                  at(_LARp,i) = at(LARpp,i);
               }
          }
        
        void GSMDecoderConverter::computeReflectionCoefficients()
          {
             using mods::utils::at;
             
             for(size_t i=0; i<_rp.size(); ++i)
               {
                  GSMInt16 temp = at(_LARp,i).abs();
                  static constexpr int segment2BaseValue = 11059;
                  static constexpr int segment3Begin = 20070;
                  if(temp < segment2BaseValue)
                    {
                       temp <<= 1;
                    }
                  else if(temp < segment3Begin)
                    {
                       temp += segment2BaseValue;
                    }
                  else
                    {
                       static constexpr int segment3Offset = 26112;
                       temp = (temp >> 2) + segment3Offset;
                    }
                  at(_rp,i) = temp;
                  if(at(_LARp,i) < 0)
                    {
                       at(_rp,i) = -at(_rp,i);
                    }
               }
          }
        
        void GSMDecoderConverter::shortTermSynthesisFiltering(int k_start, int k_end)
          {
             using mods::utils::at;
             
             for(int k=k_start; k<=k_end; ++k)
               {
                  auto sri = _drp[k];
                  for(size_t i=0; i<_rp.size(); ++i)
                    {
                       const auto idx = 7-i;
                       sri -= at(_rp,idx).mult_round(at(_v,idx));
                       at(_v,idx+1) = at(_v,idx) + at(_rp,idx).mult_round(sri);
                    }
                  at(_sr,k) = sri;
                  at(_v,0) = sri;
               }
          }
        
        void GSMDecoderConverter::deEmphasisFiltering()
          {
             using mods::utils::at;
             
             for(size_t k=0; k<_sro.size(); ++k)
               {
                  static constexpr int filterConstant = 28180;
                  auto temp = at(_sr,k) + _msr.mult_round(filterConstant);
                  _msr = temp;
                  at(_sro,k) = _msr;
               }
          }
        
        void GSMDecoderConverter::upscale()
          {
             using mods::utils::at;
             
             auto out = _decodedBuffer.slice<s16>(0, GSM_DECODED_FRAME_SAMPLES);
             
             for(int k=0; k<GSM_DECODED_FRAME_SAMPLES; ++k)
               {
                  GSMInt16 value = at(_sro,k) + at(_sro,k);
                  value >>= 3;
                  value <<= 3;
                  out[k] = value.getValue();
               }
          }
        
        auto GSMDecoderConverter::RingArray::operator[](const GSMInt16& index) -> GSMInt16&
          {
             int idx = index.getValue() + _origin;
             if(idx < 0)
               {
                  idx += _ARRAY_SIZE;
               }
             if(idx >= _ARRAY_SIZE)
               {
                  idx -= _ARRAY_SIZE;
               }
             return _array.at(idx);
          }
        
        void GSMDecoderConverter::RingArray::slideOrigin(int offset)
          {
             _origin += offset;
             if(_origin >= _ARRAY_SIZE)
               {
                  _origin -= _ARRAY_SIZE;
               }
          }
        
        auto GSMDecoderConverter::getBitsPerSampleRequirementsString() -> std::string&
          {
             static std::string requirements = "GSM codec needs bits per sample to be a multiple of 260";
             return requirements;
          }
        
     } // namespace wav
} // namespace mods
