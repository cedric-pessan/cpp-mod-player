
#include "mods/wav/GSMDecoderConverter.hpp"

namespace mods
{
   namespace wav
     {
        constexpr std::array<int, 8> GSMDecoderConverter::_larSizes;
        constexpr std::array<GSMInt16, 8> GSMDecoderConverter::_fac;
        constexpr std::array<GSMInt16, 4> GSMDecoderConverter::_qlb;
        constexpr std::array<GSMInt16, 8> GSMDecoderConverter::_mic;
        constexpr std::array<GSMInt16, 8> GSMDecoderConverter::_b;
        constexpr std::array<GSMInt16, 8> GSMDecoderConverter::_inva;
        
        GSMDecoderConverter::GSMDecoderConverter(Converter<u8>::ptr src)
          : _src(std::move(src)),
          _decodedArray {},
          _decodedBuffer(initializeArrayRWBuffer(_decodedArray)),
          _itDecodedBuffer(_decodedBuffer.RBuffer<s16>::end()),
          _encodedArray {},
          _encodedBuffer(initializeArrayRWBuffer(_encodedArray)),
          _bitReader(_encodedBuffer)
            {
            }
        
        auto GSMDecoderConverter::isFinished() const -> bool
          {
             return _itDecodedBuffer == _decodedBuffer.RBuffer<s16>::end() && _src->isFinished();
          }
        
        void GSMDecoderConverter::read(mods::utils::RWBuffer<s16>* buf)
          {
             size_t count = 0;
             while(count < buf->size())
               {
                  if(_itDecodedBuffer != _decodedBuffer.RBuffer<s16>::end())
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
                            _itDecodedBuffer = _decodedBuffer.RBuffer<s16>::begin();
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
               auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, len * sizeof(typename ARRAY::value_type), std::move(deleter));
               return mods::utils::RWBuffer<u8>(std::move(buffer)).slice<typename ARRAY::value_type>(0, len);
            }
        
        void GSMDecoderConverter::decodeGSMFrame()
          {
             if(_evenFrame)
               {
                  _src->read(&_encodedBuffer);
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
             for(size_t i=0; i<_larC.size(); ++i)
               {
                  at(_larC,i) = _bitReader.read(at(_larSizes,i));
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
             
             GSMInt16 temp1 = at(_fac, mantissa.getValue());
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
             
             const auto& brp = at(_qlb,bcr.getValue());
             
             for(size_t k=0; k<erp.size(); ++k)
               {
                  auto drpp = brp.mult_round(_drp[k - Nr]);
                  _drp[k] = at(erp,k) + drpp;
               }
             
             _drp.slideOrigin(_rpeBlockSize);
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
             larDecode();
             
             for(const auto& param : shortTermSynthesisParameters)
               {
                  switch(param.kRange)
                    {
                     case ShortTermSynthesisFilteringRanges::range_0_12:
                       larInterpolation_0_12();
                       break;
                     case ShortTermSynthesisFilteringRanges::range_13_26:
                       larInterpolation_13_26();
                       break;
                     case ShortTermSynthesisFilteringRanges::range_27_39:
                       larInterpolation_27_39();
                       break;
                     case ShortTermSynthesisFilteringRanges::range_40_159:
                       larInterpolation_40_159();
                       break;
                    }
                  computeReflectionCoefficients();
                  shortTermSynthesisFiltering(param.startk, param.endk);
               }
          }
        
        void GSMDecoderConverter::larDecode()
          {
             using mods::utils::at;
             
             _currentLarPP = 1 - _currentLarPP;
             auto& larPP = at(_larPP, _currentLarPP);
             
             static constexpr int scaleShift = 10;
             
             for(size_t i=0; i<larPP.size(); ++i)
               {
                  GSMInt16 temp1 = (at(_larC,i) + at(_mic,i)) << scaleShift;
                  GSMInt16 temp2 = at(_b,i) << 1;
                  temp1 -= temp2;
                  temp1 = at(_inva,i).mult_round(temp1);
                  at(larPP,i) = temp1 + temp1;
               }
          }
        
        void GSMDecoderConverter::larInterpolation_0_12()
          {
             using mods::utils::at;
             auto& larPPOld = at(_larPP,1-_currentLarPP);
             auto& larPP = at(_larPP,_currentLarPP);
             
             for(int i=0; i<_numberOfLarCoefficients; ++i)
               {
                  at(_larP,i) = (at(larPPOld,i) >> 2) + (at(larPP,i) >> 2);
                  at(_larP,i) += (at(larPPOld,i) >> 1);
               }
          }
        
        void GSMDecoderConverter::larInterpolation_13_26()
          {
             using mods::utils::at;
             auto& larPPOld = at(_larPP,1-_currentLarPP);
             auto& larPP = at(_larPP,_currentLarPP);
             
             for(int i=0; i<_numberOfLarCoefficients; ++i)
               {
                  at(_larP,i) = (at(larPPOld,i) >> 1) + (at(larPP,i) >> 1);
               }
          }
        
        void GSMDecoderConverter::larInterpolation_27_39()
          {
             using mods::utils::at;
             auto& larPPOld = at(_larPP,1-_currentLarPP);
             auto& larPP = at(_larPP,_currentLarPP);
             
             for(int i=0; i<_numberOfLarCoefficients; ++i)
               {
                  at(_larP,i) = (at(larPPOld,i) >> 2) + (at(larPP,i) >> 2);
                  at(_larP,i) += (at(larPP,i) >> 1);
               }
          }
        
        void GSMDecoderConverter::larInterpolation_40_159()
          {
             using mods::utils::at;
             auto& larPP = at(_larPP,_currentLarPP);
             
             for(int i=0; i<_numberOfLarCoefficients; ++i)
               {
                  at(_larP,i) = at(larPP,i);
               }
          }
        
        void GSMDecoderConverter::computeReflectionCoefficients()
          {
             using mods::utils::at;
             
             for(size_t i=0; i<_rp.size(); ++i)
               {
                  GSMInt16 temp = at(_larP,i).abs();
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
                  if(at(_larP,i) < 0)
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
             
             for(int k=0; k<GSM_DECODED_FRAME_SIZE; ++k)
               {
                  GSMInt16 value = at(_sro,k) + at(_sro,k);
                  value >>= 3;
                  value <<= 3;
                  _decodedBuffer[k] = value.getValue();
               }
          }
        
        auto GSMDecoderConverter::RingArray::operator[](const GSMInt16& index) -> GSMInt16&
          {
             int idx = index.getValue() + _origin;
             if(idx < 0)
               {
                  idx += _ringArraySize;
               }
             if(idx >= _ringArraySize)
               {
                  idx -= _ringArraySize;
               }
             return _array.at(idx);
          }
        
        void GSMDecoderConverter::RingArray::slideOrigin(int offset)
          {
             _origin += offset;
             if(_origin >= _ringArraySize)
               {
                  _origin -= _ringArraySize;
               }
          }
        
        auto GSMDecoderConverter::getBitsPerSampleRequirementsString() -> std::string&
          {
             static std::string requirements = "GSM codec needs bits per sample to be a multiple of 260";
             return requirements;
          }
        
     } // namespace wav
} // namespace mods
