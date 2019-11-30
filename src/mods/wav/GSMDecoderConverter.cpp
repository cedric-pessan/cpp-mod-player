
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
        
        bool GSMDecoderConverter::isFinished() const
          {
             return _itDecodedBuffer == _decodedBuffer.RBuffer<u8>::end() && _src->isFinished();
          }
        
        void GSMDecoderConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             int count = 0;
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
          mods::utils::RWBuffer<typename ARRAY::value_type> GSMDecoderConverter::initializeArrayRWBuffer(ARRAY& backArray)
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
             for(int i=0; i<8; ++i)
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
                  for(int j=0; j<13; ++j)
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
             if(xmax > 15)
               {
                  exponent = (xmax >> 3) - 1;
               }
             GSMInt16 mantissa = xmax - (exponent << 3);
             
             if(mantissa == 0)
               {
                  exponent = -4;
                  mantissa = 15;
               }
             else
               {
                  bool itest = false;
                  for(int i=0; i<3; ++i)
                    {
                       if(mantissa > 7)
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
             mantissa -= 8;
             
             GSMInt16 temp1 = at(_FAC, mantissa.getValue());
             GSMInt16 temp2 = 6 - exponent;
             GSMInt16 temp3 = 1 << (temp2 - 1);
             
             auto& x = at(_subframes, subframe).x;
             auto& xp = at(_subframes, subframe).xp;
             
             for(int i=0; i <13; ++i)
               {
                  GSMInt16 temp = (at(x,i) << 1) - 7;
                  temp <<= 12;
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
             
             for(int k=0; k<40; ++k)
               {
                  at(ep,k) = 0;
               }
             for(int i=0; i<13; ++i)
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
             auto Nr = Ncr;
             if(Ncr < 40)
               {
                  Nr = _nrp;
               }
             if(Ncr > 120)
               {
                  Nr = _nrp;
               }
             _nrp = Nr;
             
             auto& brp = at(_QLB,bcr.getValue());
             
             for(int k=0; k<40; ++k)
               {
                  auto drpp = brp.mult_round(_drp[k - Nr]);
                  _drp[k] = at(erp,k) + drpp;
               }
             
             _drp.slideOrigin(40);
          }
        
        void GSMDecoderConverter::shortTermSynthesis()
          {
             LARDecode();
             
             LARInterpolation_0_12();
             computeReflectionCoefficients();
             shortTermSynthesisFiltering(0, 12);
             
             LARInterpolation_13_26();
             computeReflectionCoefficients();
             shortTermSynthesisFiltering(13, 26);
             
             LARInterpolation_27_39();
             computeReflectionCoefficients();
             shortTermSynthesisFiltering(27, 39);
             
             LARInterpolation_40_159();
             computeReflectionCoefficients();
             shortTermSynthesisFiltering(40, 159);
          }
        
        void GSMDecoderConverter::LARDecode()
          {
             using mods::utils::at;
             
             _currentLARpp = 1 - _currentLARpp;
             
             for(int i=0; i<8; ++i)
               {
                  GSMInt16 temp1 = (at(_LARc,i) + at(_MIC,i)) << 10;
                  GSMInt16 temp2 = at(_B,i) << 1;
                  temp1 -= temp2;
                  temp1 = at(_INVA,i).mult_round(temp1);
                  auto& LARpp = at(_LARpp, _currentLARpp);
                  at(LARpp,i) = temp1 + temp1;
               }
          }
        
        void GSMDecoderConverter::LARInterpolation_0_12()
          {
             using mods::utils::at;
             auto& LARppOld = at(_LARpp,1-_currentLARpp);
             auto& LARpp = at(_LARpp,_currentLARpp);
             
             for(int i=0; i<8; ++i)
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
             
             for(int i=0; i<8; ++i)
               {
                  at(_LARp,i) = (at(LARppOld,i) >> 1) + (at(LARpp,i) >> 1);
               }
          }
        
        void GSMDecoderConverter::LARInterpolation_27_39()
          {
             using mods::utils::at;
             auto& LARppOld = at(_LARpp,1-_currentLARpp);
             auto& LARpp = at(_LARpp,_currentLARpp);
             
             for(int i=0; i<8; ++i)
               {
                  at(_LARp,i) = (at(LARppOld,i) >> 2) + (at(LARpp,i) >> 2);
                  at(_LARp,i) += (at(LARpp,i) >> 1);
               }
          }
        
        void GSMDecoderConverter::LARInterpolation_40_159()
          {
             using mods::utils::at;
             auto& LARpp = at(_LARpp,_currentLARpp);
             
             for(int i=0; i<8; ++i)
               {
                  at(_LARp,i) = at(LARpp,i);
               }
          }
        
        void GSMDecoderConverter::computeReflectionCoefficients()
          {
             using mods::utils::at;
             
             for(int i=0; i<8; ++i)
               {
                  GSMInt16 temp = at(_LARp,i).abs();
                  if(temp < 11059)
                    {
                       temp <<= 1;
                    }
                  else if(temp < 20070)
                    {
                       temp += 11059;
                    }
                  else
                    {
                       temp = (temp >> 2) + 26112;
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
                  for(int i=0; i<8; ++i)
                    {
                       sri -= at(_rp,7-i).mult_round(at(_v,7-i));
                       at(_v,8-i) = at(_v,7-i) + at(_rp,7-i).mult_round(sri);
                    }
                  at(_sr,k) = sri;
                  at(_v,0) = sri;
               }
          }
        
        void GSMDecoderConverter::deEmphasisFiltering()
          {
             using mods::utils::at;
             
             for(int k=0; k<160; ++k)
               {
                  auto temp = at(_sr,k) + _msr.mult_round(28180);
                  _msr = temp;
                  at(_sro,k) = _msr;
               }
          }
        
        void GSMDecoderConverter::upscale()
          {
             using mods::utils::at;
             
             auto out = _decodedBuffer.slice<s16>(0, 160);
             
             for(int k=0; k<160; ++k)
               {
                  GSMInt16 value = at(_sro,k) + at(_sro,k);
                  value >>= 3;
                  value <<= 3;
                  out[k] = value.getValue();
               }
          }
        
        GSMInt16& GSMDecoderConverter::RingArray::operator[](const GSMInt16& index)
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
        
     } // namespace wav
} // namespace mods
