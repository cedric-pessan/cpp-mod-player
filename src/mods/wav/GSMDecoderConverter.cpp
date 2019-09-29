
#include "mods/wav/GSMDecoderConverter.hpp"

namespace mods
{
   namespace wav
     {
        constexpr std::array<int, 8> GSMDecoderConverter::_LAR_SIZES_array;
        constexpr std::array<GSMInt16, 8> GSMDecoderConverter::_FAC_array;
        constexpr std::array<GSMInt16, 4> GSMDecoderConverter::_QLB_array;
        constexpr std::array<GSMInt16, 8> GSMDecoderConverter::_MIC_array;
        constexpr std::array<GSMInt16, 8> GSMDecoderConverter::_B_array;
        constexpr std::array<GSMInt16, 8> GSMDecoderConverter::_INVA_array;
        
        GSMDecoderConverter::GSMDecoderConverter(WavConverter::ptr src)
          : _src(std::move(src)),
          _decodedArray {},
          _decodedBuffer(initializeArrayRWBuffer(_decodedArray)),
          _itDecodedBuffer(_decodedBuffer.RBuffer<u8>::end()),
          _encodedArray {},
          _encodedBuffer(initializeArrayRWBuffer(_encodedArray)),
          _bitReader(_encodedBuffer),
          _LARc(initializeArrayRWBuffer(_LARc_array)),
          _LARppBufferArray 
          {
             initializeArrayRWBuffer(_LARppArrays[0]),
               initializeArrayRWBuffer(_LARppArrays[1])
          },
          _LARpp(initializeArrayRWBuffer(_LARppBufferArray)),
          _LARp(initializeArrayRWBuffer(_LARpArray)),
          _rp(initializeArrayRWBuffer(_rpArray)),
          _v(initializeArrayRWBuffer(_vArray)),
          _sr(initializeArrayRWBuffer(_srArray)),
          _sro(initializeArrayRWBuffer(_sroArray)),
          _LAR_SIZES(initializeArrayRBuffer(_LAR_SIZES_array)),
          _FAC(initializeArrayRBuffer(_FAC_array)),
          _QLB(initializeArrayRBuffer(_QLB_array)),
          _MIC(initializeArrayRBuffer(_MIC_array)),
          _B(initializeArrayRBuffer(_B_array)),
          _INVA(initializeArrayRBuffer(_INVA_array)),
          _subframes(initializeArrayRWBuffer(_subframesArray))
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
        
        template<typename ARRAY>
          mods::utils::RBuffer<typename ARRAY::value_type> GSMDecoderConverter::initializeArrayRBuffer(const ARRAY& backArray)
            {
               auto* ptr = static_cast<const u8*>(static_cast<const void*>(backArray.data()));
               auto len = backArray.size();
               auto deleter = std::make_unique<mods::utils::RBufferBackend::EmptyDeleter>();
               auto buffer = std::make_shared<mods::utils::RBufferBackend>(ptr, len * sizeof(typename ARRAY::value_type), std::move(deleter));
               return mods::utils::RBuffer<u8>(buffer).slice<typename ARRAY::value_type>(0, len);
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
             for(int i=0; i<8; ++i)
               {
                  _LARc[i] = _bitReader.read(_LAR_SIZES[i]);
               }
             
             for(int i=0; i<4; ++i)
               {
                  _subframes[i].N = _bitReader.read(LTP_LAG_SIZE);
                  _subframes[i].b = _bitReader.read(LTP_GAIN_SIZE);
                  
                  _subframes[i].M = _bitReader.read(RPE_GRID_POSITION_SIZE);
                  _subframes[i].Xmax = _bitReader.read(RPE_BLOCK_AMPLITUDE_SIZE);
                  for(int j=0; j<13; ++j)
                    {
                       _subframes[i].x[j] = _bitReader.read(RPE_PULSE_SIZE);
                    }
               }
          }
        
        void GSMDecoderConverter::inverseAPCM(int subframe)
          {
             GSMInt16 exponent = 0;
             auto xmax = _subframes[subframe].Xmax;
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
             
             GSMInt16 temp1 = _FAC[mantissa.getValue()];
             GSMInt16 temp2 = 6 - exponent;
             GSMInt16 temp3 = 1 << (temp2 - 1);
             
             auto& x = _subframes[subframe].x;
             auto& xp = _subframes[subframe].xp;
             
             for(int i=0; i <13; ++i)
               {
                  GSMInt16 temp = (x[i] << 1) - 7;
                  temp <<= 12;
                  temp = temp.mult_round(temp1);
                  temp += temp3;
                  xp[i] = temp >> temp2;
               }
          }
        
        void GSMDecoderConverter::RPEGridPosition(int subframe)
          {
             auto& ep = _subframes[subframe].ep;
             auto& M = _subframes[subframe].M;
             auto& xp = _subframes[subframe].xp;
             
             for(int k=0; k<40; ++k)
               {
                  ep[k] = 0;
               }
             for(int i=0; i<13; ++i)
               {
                  ep[M.getValue() + (3*i)] = xp[i];
               }
          }
        
        void GSMDecoderConverter::longTermPrediction(int subframe)
          {
             auto& Ncr = _subframes[subframe].N;
             auto& bcr = _subframes[subframe].b;
             auto& erp = _subframes[subframe].ep;
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
             
             auto& brp = _QLB[bcr.getValue()];
             
             for(int k=0; k<40; ++k)
               {
                  auto drpp = brp.mult_round(_drp[k - Nr]);
                  auto dumped = erp[k] + drpp;
                  _drp[k] = erp[k] + drpp;
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
             _currentLARpp = 1 - _currentLARpp;
             
             for(int i=0; i<8; ++i)
               {
                  GSMInt16 temp1 = (_LARc[i] + _MIC[i]) << 10;
                  GSMInt16 temp2 = _B[i] << 1;
                  temp1 -= temp2;
                  temp1 = _INVA[i].mult_round(temp1);
                  _LARpp[_currentLARpp][i] = temp1 + temp1;
               }
          }
        
        void GSMDecoderConverter::LARInterpolation_0_12()
          {
             auto& LARppOld = _LARpp[1-_currentLARpp];
             auto& LARpp = _LARpp[_currentLARpp];
             
             for(int i=0; i<8; ++i)
               {
                  _LARp[i] = (LARppOld[i] >> 2) + (LARpp[i] >> 2);
                  _LARp[i] += (LARppOld[i] >> 1);
               }
          }
        
        void GSMDecoderConverter::LARInterpolation_13_26()
          {
             auto& LARppOld = _LARpp[1-_currentLARpp];
             auto& LARpp = _LARpp[_currentLARpp];
             
             for(int i=0; i<8; ++i)
               {
                  _LARp[i] = (LARppOld[i] >> 1) + (LARpp[i] >> 1);
               }
          }
        
        void GSMDecoderConverter::LARInterpolation_27_39()
          {
             auto& LARppOld = _LARpp[1-_currentLARpp];
             auto& LARpp = _LARpp[_currentLARpp];
             
             for(int i=0; i<8; ++i)
               {
                  _LARp[i] = (LARppOld[i] >> 2) + (LARpp[i] >> 2);
                  _LARp[i] += (LARpp[i] >> 1);
               }
          }
        
        void GSMDecoderConverter::LARInterpolation_40_159()
          {
             auto& LARpp = _LARpp[_currentLARpp];
             
             for(int i=0; i<8; ++i)
               {
                  _LARp[i] = LARpp[i];
               }
          }
        
        void GSMDecoderConverter::computeReflectionCoefficients()
          {
             for(int i=0; i<8; ++i)
               {
                  GSMInt16 temp = _LARp[i].abs();
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
                  _rp[i] = temp;
                  if(_LARp[i] < 0)
                    {
                       _rp[i] = -_rp[i];
                    }
               }
          }
        
        void GSMDecoderConverter::shortTermSynthesisFiltering(int k_start, int k_end)
          {
             for(int k=k_start; k<=k_end; ++k)
               {
                  auto sri = _drp[k];
                  for(int i=0; i<8; ++i)
                    {
                       sri -= _rp[7-i].mult_round(_v[7-i]);
                       _v[8-i] = _v[7-i] + _rp[7-i].mult_round(sri);
                    }
                  _sr[k] = sri;
                  _v[0] = sri;
               }
          }
        
        void GSMDecoderConverter::deEmphasisFiltering()
          {
             for(int k=0; k<160; ++k)
               {
                  auto temp = _sr[k] + _msr.mult_round(28180);
                  _msr = temp;
                  _sro[k] = _msr;
               }
          }
        
        void GSMDecoderConverter::upscale()
          {
             auto out = _decodedBuffer.slice<s16>(0, 160);
             
             for(int k=0; k<160; ++k)
               {
                  GSMInt16 value = _sro[k] + _sro[k];
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
        
        GSMDecoderConverter::SubFrame::SubFrame()
          : x(GSMDecoderConverter::initializeArrayRWBuffer(_xArray)),
          xp(GSMDecoderConverter::initializeArrayRWBuffer(_xpArray)),
          ep(GSMDecoderConverter::initializeArrayRWBuffer(_epArray))
            {
            }
        
     } // namespace wav
} // namespace mods
