
#include "mods/utils/arithmeticShifter.hpp"
#include "mods/wav/TruspeechDecoderConverter.hpp"

/*
 * based on ffmpeg source algorithm
 */

namespace mods
{
   namespace wav
     {
        TruspeechDecoderConverter::TruspeechDecoderConverter(Converter<u8>::ptr src)
          : _src(std::move(src)),
          _decodedArray {},
          _decodedBuffer(initializeArrayRWBuffer(_decodedArray)),
          _itDecodedBuffer(_decodedBuffer.RBuffer<s16>::end()),
          _subframes
          {
             _decodedBuffer.slice<s16>(0,                   _subframeLength),
             _decodedBuffer.slice<s16>(_subframeLength,     _subframeLength),
             _decodedBuffer.slice<s16>(_subframeLength * 2, _subframeLength),
             _decodedBuffer.slice<s16>(_subframeLength * 3, _subframeLength)
          },
          _encodedArray {},
          _encodedBuffer(initializeArrayRWBuffer(_encodedArray)),
          _bitReader(_encodedBuffer),
          _vector {},
          _offset1 {},
          _offset2 {},
          _pulseval {},
          _pulsepos {},
          _pulseoff {},
          _correlatedVectors {},
          _filters 
          {
               {
                    {}, {}, {}, {}
               }
          },
          _filterBuffer {},
          _newVector {},
          _tmp1 {},
          _tmp2 {},
          _tmp3 {}
          {
          }
        
        template<typename ARRAY>
          auto TruspeechDecoderConverter::initializeArrayRWBuffer(ARRAY& backArray) -> mods::utils::RWBuffer<typename ARRAY::value_type>
            {
               auto* ptr = static_cast<u8*>(static_cast<void*>(backArray.data()));
               auto len = backArray.size();
               auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
               auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, len * sizeof(typename ARRAY::value_type), std::move(deleter));
               return mods::utils::RWBuffer<u8>(std::move(buffer)).slice<typename ARRAY::value_type>(0, len);
            }
        
        auto TruspeechDecoderConverter::isFinished() const -> bool
          {
             return _itDecodedBuffer == _decodedBuffer.RBuffer<s16>::end() && _src->isFinished();
          }
        
        void TruspeechDecoderConverter::read(mods::utils::RWBuffer<s16>* buf)
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
                            decodeTruspeechFrame();
                            _itDecodedBuffer = _decodedBuffer.RBuffer<s16>::begin();
                         }
                    }
               }
          }
        
        void TruspeechDecoderConverter::decodeTruspeechFrame()
          {
             _src->read(&_encodedBuffer);
             _bitReader.reset();
             
             readParameters();
             
             correlateFilter();
             filtersMerge();
             
             for(int i=0; i<4; ++i)
               {
                  applyTwoPointFilter(i);
                  placePulses(i);
                  updateFilters(i);
                  synth(i);
               }
             
             _currentCorrelatedVector = 1 - _currentCorrelatedVector;
          }
        
        namespace
          {
             template<int number, int size>
               struct CodeBook
               {
                  std::array<s16, size> array;
                  
                  static constexpr auto getArrayMaskSize() -> int
                    {
                       static_assert(size > 0, "Codebook size should be positive");
                       u32 value = size;
                       int maskSize = 0;
                       while(value > 0)
                         {
                            if((value & 1U) != 0)
                              {
                                 value >>= 1U;
                                 assert(value == 0);
                                 return maskSize;
                              }
                            value >>= 1U;
                            ++maskSize;
                         }
                       return maskSize;
                    }
                  
                  template<typename DESTARRAY, typename BITREADER>
                    void read(DESTARRAY* out, BITREADER* bitReader) const
                      {
                         using mods::utils::at;
                         at(*out, number) = at(array, bitReader->read(getArrayMaskSize()));
                      }
               };
             
             constexpr CodeBook<0,32> codeBook0
               {
                    {
                       s16(0x8240), s16(0x8364), s16(0x84CE), s16(0x865D), s16(0x8805), s16(0x89DE), s16(0x8BD7), s16(0x8DF4),
                         s16(0x9051), s16(0x92E2), s16(0x95DE), s16(0x990F), s16(0x9C81), s16(0xA079), s16(0xA54C), s16(0xAAD2),
                         s16(0xB18A), s16(0xB90A), s16(0xC124), s16(0xC9CC), s16(0xD339), s16(0xDDD3), s16(0xE9D6), s16(0xF893),
                         s16(0x096F), s16(0x1ACA), s16(0x29EC), s16(0x381F), s16(0x45F9), s16(0x546A), s16(0x63C3), s16(0x73B5)
                    }
               };
             
             constexpr CodeBook<1,32> codeBook1
               {
                    {
                       s16(0x9F65), s16(0xB56B), s16(0xC583), s16(0xD371), s16(0xE018), s16(0xEBB4), s16(0xF61C), s16(0xFF59),
                         s16(0x085B), s16(0x1106), s16(0x1952), s16(0x214A), s16(0x28C9), s16(0x2FF8), s16(0x36E6), s16(0x3D92),
                         s16(0x43DF), s16(0x49BB), s16(0x4F46), s16(0x5467), s16(0x5930), s16(0x5DA3), s16(0x61EC), s16(0x65F9),
                         s16(0x69D4), s16(0x6D5A), s16(0x709E), s16(0x73AD), s16(0x766B), s16(0x78F0), s16(0x7B5A), s16(0x7DA5)
                    }
               };
             
             constexpr CodeBook<2,16> codeBook2
               {
                    {
                       s16(0x96F8), s16(0xA3B4), s16(0xAF45), s16(0xBA53), s16(0xC4B1), s16(0xCECC), s16(0xD86F), s16(0xE21E),
                         s16(0xEBF3), s16(0xF640), s16(0x00F7), s16(0x0C20), s16(0x1881), s16(0x269A), s16(0x376B), s16(0x4D60)
                    }
               };
             
             constexpr CodeBook<3,16> codeBook3
               {
                    {
                       s16(0xC654), s16(0xDEF2), s16(0xEFAA), s16(0xFD94), s16(0x096A), s16(0x143F), s16(0x1E7B), s16(0x282C),
                         s16(0x3176), s16(0x3A89), s16(0x439F), s16(0x4CA2), s16(0x557F), s16(0x5E50), s16(0x6718), s16(0x6F8D)
                    }
               };
             
             constexpr CodeBook<4,16> codeBook4
               {
                    {
                       s16(0xABE7), s16(0xBBA8), s16(0xC81C), s16(0xD326), s16(0xDD0E), s16(0xE5D4), s16(0xEE22), s16(0xF618),
                         s16(0xFE28), s16(0x064F), s16(0x0EB7), s16(0x17B8), s16(0x21AA), s16(0x2D8B), s16(0x3BA2), s16(0x4DF9)
                    }
               };
             
             constexpr CodeBook<5,8> codeBook5
               {
                    {
                       s16(0xD51B), s16(0xF12E), s16(0x042E), s16(0x13C7), s16(0x2260), s16(0x311B), s16(0x40DE), s16(0x5385)
                    }
               };
             
             constexpr CodeBook<6,8> codeBook6
               {
                  {
                     s16(0xB550), s16(0xC825), s16(0xD980), s16(0xE997), s16(0xF883), s16(0x0752), s16(0x1811), s16(0x2E18)
                  }
               };
             
             constexpr CodeBook<7,8> codeBook7
               {
                    {
                       s16(0xCEF0), s16(0xE4F9), s16(0xF6BB), s16(0x0646), s16(0x14F5), s16(0x23FF), s16(0x356F), s16(0x4A8D)
                    }
               };
             
             constexpr std::array<s16,8> decay_994_1000
               {
                  0x7F3B, 0x7E78, 0x7DB6, 0x7CF5, 0x7C35, 0x7B76, 0x7AB8, 0x79FC 
               };
             
             using Order2Coeffs = std::array<s16, 2>;
             constexpr std::array<Order2Coeffs, 25> order2Coeffs
               {{
                  { s16(0xED2F), s16(0x5239) },
                  { s16(0x54F1), s16(0xE4A9) },
                  { s16(0x2620), s16(0xEE3E) },
                  { s16(0x09D6), s16(0x2C40) },
                  { s16(0xEFB5), s16(0x2BE0) },
                  
                  { s16(0x3FE1), s16(0x3339) },
                  { s16(0x442F), s16(0xE6FE) },
                  { s16(0x4458), s16(0xF9DF) },
                  { s16(0xF231), s16(0x43DB) },
                  { s16(0x3DB0), s16(0xF705) },
                  
                  { s16(0x4F7B), s16(0xFEFB) },
                  { s16(0x26AD), s16(0x0CDC) },
                  { s16(0x33C2), s16(0x0739) },
                  { s16(0x12BE), s16(0x43A2) },
                  { s16(0x1BDF), s16(0x1F3E) },
                  
                  { s16(0x0211), s16(0x0796) },
                  { s16(0x2AEB), s16(0x163F) },
                  { s16(0x050D), s16(0x3A38) },
                  { s16(0x0D1E), s16(0x0D78) },
                  { s16(0x150F), s16(0x3346) },
                  
                  { s16(0x38A4), s16(0x0B7D) },
                  { s16(0x2D5D), s16(0x1FDF) },
                  { s16(0x19B7), s16(0x2822) },
                  { s16(0x0D99), s16(0x1F12) },
                  { s16(0x194C), s16(0x0CE6) }
               }};
             
             constexpr std::array<s16,64> pulseScales 
               {                  
                  s16(0x0002), s16(0x0006), s16(0xFFFE), s16(0xFFFA),
                  s16(0x0004), s16(0x000C), s16(0xFFFC), s16(0xFFF4),
                  s16(0x0006), s16(0x0012), s16(0xFFFA), s16(0xFFEE),
                  s16(0x000A), s16(0x001E), s16(0xFFF6), s16(0xFFE2),
                  s16(0x0010), s16(0x0030), s16(0xFFF0), s16(0xFFD0),
                  s16(0x0019), s16(0x004B), s16(0xFFE7), s16(0xFFB5),
                  s16(0x0028), s16(0x0078), s16(0xFFD8), s16(0xFF88),
                  s16(0x0040), s16(0x00C0), s16(0xFFC0), s16(0xFF40),
                  s16(0x0065), s16(0x012F), s16(0xFF9B), s16(0xFED1),
                  s16(0x00A1), s16(0x01E3), s16(0xFF5F), s16(0xFE1D),
                  s16(0x0100), s16(0x0300), s16(0xFF00), s16(0xFD00),
                  s16(0x0196), s16(0x04C2), s16(0xFE6A), s16(0xFB3E),
                  s16(0x0285), s16(0x078F), s16(0xFD7B), s16(0xF871),
                  s16(0x0400), s16(0x0C00), s16(0xFC00), s16(0xF400),
                  s16(0x0659), s16(0x130B), s16(0xF9A7), s16(0xECF5),
                  s16(0x0A14), s16(0x1E3C), s16(0xF5EC), s16(0xE1C4)
               };
             
             constexpr std::array<s16,120> pulseValues
               {
                  0x0E46, 0x0CCC, 0x0B6D, 0x0A28, 0x08FC, 0x07E8, 0x06EB, 0x0604,
                    0x0532, 0x0474, 0x03C9, 0x0330, 0x02A8, 0x0230, 0x01C7, 0x016C,
                    0x011E, 0x00DC, 0x00A5, 0x0078, 0x0054, 0x0038, 0x0023, 0x0014,
                    0x000A, 0x0004, 0x0001, 0x0000, 0x0000, 0x0000,
                  
                    0x0196, 0x017A, 0x015F, 0x0145, 0x012C, 0x0114, 0x00FD, 0x00E7,
                    0x00D2, 0x00BE, 0x00AB, 0x0099, 0x0088, 0x0078, 0x0069, 0x005B,
                    0x004E, 0x0042, 0x0037, 0x002D, 0x0024, 0x001C, 0x0015, 0x000F,
                    0x000A, 0x0006, 0x0003, 0x0001, 0x0000, 0x0000,
                    
                    0x001D, 0x001C, 0x001B, 0x001A, 0x0019, 0x0018, 0x0017, 0x0016,
                    0x0015, 0x0014, 0x0013, 0x0012, 0x0011, 0x0010, 0x000F, 0x000E,
                    0x000D, 0x000C, 0x000B, 0x000A, 0x0009, 0x0008, 0x0007, 0x0006,
                    0x0005, 0x0004, 0x0003, 0x0002, 0x0001, 0x0000,
                    
                    0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
                    0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
                    0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
                    0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001
               };
             
             constexpr std::array<s16,8> decay_35_64
               {
                  0x4666, 0x26B8, 0x154C, 0x0BB6, 0x0671, 0x038B, 0x01F3, 0x0112
               };
             
             constexpr std::array<s16,8> decay_3_4
               {
                  0x6000, 0x4800, 0x3600, 0x2880, 0x1E60, 0x16C8, 0x1116, 0x0CD1
               };
          } // namespace
        
        void TruspeechDecoderConverter::readParameters()
          {
             using mods::utils::at;
             
             static constexpr int pulsevalSize = 14;
             static constexpr int pulseposSize = 27;
             static constexpr int offset2Size = 7;
             
             codeBook7.read(&_vector, &_bitReader);
             codeBook6.read(&_vector, &_bitReader);
             codeBook5.read(&_vector, &_bitReader);
             codeBook4.read(&_vector, &_bitReader);
             codeBook3.read(&_vector, &_bitReader);
             codeBook2.read(&_vector, &_bitReader);
             codeBook1.read(&_vector, &_bitReader);
             codeBook0.read(&_vector, &_bitReader);
             
             _flag = _bitReader.read(1) != 0;
             
             u32 offset1_0 = _bitReader.read(4) << 4U;
             at(_offset2,3) = _bitReader.read(offset2Size);
             at(_offset2,2) = _bitReader.read(offset2Size);
             at(_offset2,1) = _bitReader.read(offset2Size);
             at(_offset2,0) = _bitReader.read(offset2Size);
             
             u32 offset1_1 = _bitReader.read(4);
             at(_pulseval,1) = _bitReader.read(pulsevalSize);
             at(_pulseval,0) = _bitReader.read(pulsevalSize);
             
             offset1_1 |= _bitReader.read(4) << 4U;
             at(_pulseval,3) = _bitReader.read(pulsevalSize);
             at(_pulseval,2) = _bitReader.read(pulsevalSize);
             
             offset1_0 |= _bitReader.read(1);
             at(_pulsepos,0) = _bitReader.read(pulseposSize);
             at(_pulseoff,0) = _bitReader.read(4);
             
             offset1_0 |= _bitReader.read(1) << 1U;
             at(_pulsepos,1) = _bitReader.read(pulseposSize);
             at(_pulseoff,1) = _bitReader.read(4);
             
             offset1_0 |= _bitReader.read(1) << 2U;
             at(_pulsepos,2) = _bitReader.read(pulseposSize);
             at(_pulseoff,2) = _bitReader.read(4);
             
             offset1_0 |= _bitReader.read(1) << 3U;
             at(_pulsepos,3) = _bitReader.read(pulseposSize);
             at(_pulseoff,3) = _bitReader.read(4);
             
             at(_offset1,0) = offset1_0;
             at(_offset1,1) = offset1_1;
          }
        
        void TruspeechDecoderConverter::correlateFilter()
          {
             using mods::utils::at;
             using mods::utils::arithmeticShifter::shiftRight;
             
             std::array<s16,_numberOfFilterCoefficients> tmp {};
             auto& correlatedVector = at(_correlatedVectors, _currentCorrelatedVector);
             
             static constexpr int roundConstant = 0x4000;
             static constexpr int fixedPointShift = 15;
             
             for(size_t i=0; i<correlatedVector.size(); ++i)
               {
                  if(i > 0)
                    {
		       std::copy(correlatedVector.begin(), correlatedVector.begin() + i, tmp.begin());
                       for(size_t j=0; j<i; ++j)
                         {
                            at(correlatedVector,j) += shiftRight(at(tmp, i - j -1) * at(_vector,i) + roundConstant, fixedPointShift);
                         }
                    }
                  static constexpr int offsetVector = 8;
                  at(correlatedVector,i) = shiftRight(offsetVector - at(_vector,i), 3);
               }
             for(size_t i=0; i<correlatedVector.size(); ++i)
               {
                  at(correlatedVector,i) = shiftRight(at(correlatedVector,i) * at(decay_994_1000,i), fixedPointShift);
               }
             
             _filtVal = at(_vector,0);
          }
        
        void TruspeechDecoderConverter::filtersMerge()
          {
             using mods::utils::at;
             using mods::utils::arithmeticShifter::shiftRight;
             
             auto& previousCorrelatedVector = at(_correlatedVectors, 1 - _currentCorrelatedVector);
             auto& correlatedVector = at(_correlatedVectors, _currentCorrelatedVector);
             
             int currentFilter = 0;
             auto& filter1 = at(_filters, currentFilter++);
             auto& filter2 = at(_filters, currentFilter++);
             auto& filter3 = at(_filters, currentFilter++);
             auto& filter4 = at(_filters, currentFilter++);
             
             if(!_flag)
               {
                  for(size_t i=0; i<previousCorrelatedVector.size(); ++i)
                    {
                       at(filter1, i) = at(previousCorrelatedVector, i);
                       at(filter2, i) = at(previousCorrelatedVector, i);
                    }
               }
             else
               {
                  static constexpr int mixCoef1 = 21846;
                  static constexpr int mixCoef2 = 10923;
                  static constexpr int roundConstant = 16384;
                  static constexpr int fixedPointShift = 15;
                  
                  for(size_t i=0; i<correlatedVector.size(); ++i)
                    {
                       at(filter1, i) = shiftRight(at(correlatedVector, i) * mixCoef1 + at(previousCorrelatedVector, i) * mixCoef2 + roundConstant, fixedPointShift);
                       at(filter2, i) = shiftRight(at(correlatedVector, i) * mixCoef2 + at(previousCorrelatedVector, i) * mixCoef1 + roundConstant, fixedPointShift);
                    }
               }
             
             for(size_t i=0; i<correlatedVector.size(); ++i)
               {
                  at(filter3, i) = at(correlatedVector, i);
                  at(filter4, i) = at(correlatedVector, i);
               }
          }
        
        void TruspeechDecoderConverter::applyTwoPointFilter(int subframe)
          {
             using mods::utils::at;
             using mods::utils::arithmeticShifter::shiftRight;
             
             static constexpr int clearFilterSpecialCase = 127;
             static constexpr int offset2Factor = 25;
             static constexpr int baseOffset = 18;
             
             std::array<s16, _filterBufferLength + _subframeLength> tmp {};
             
             int t = at(_offset2, subframe);
             if(t == clearFilterSpecialCase)
	       {
		  std::fill(_newVector.begin(), _newVector.end(), 0);
		  return;
	       }
             for(size_t i=0; i<_filterBuffer.size(); ++i)
               {
                  at(tmp,i) = at(_filterBuffer,i);
               }
             s32 off = (t / offset2Factor) + at(_offset1, shiftRight(subframe, 1)) + baseOffset;
             off = mods::utils::clamp(off, 0, _filterBufferLength-1);
             size_t idx0 = _filterBufferLength - 1 - off;
             size_t idx1 = _filterBuffer.size();
             const auto& filter = at(order2Coeffs, t % order2Coeffs.size());
             
             static constexpr int roundConstant = 0x2000;
             static constexpr int fixedPointShift = 14;
             for(size_t i=0; i<_newVector.size(); ++i,++idx1)
               {
                  s16 v = shiftRight(at(tmp, idx0)   * at(filter,0) + 
                                     at(tmp, idx0+1) * at(filter,1) + roundConstant, fixedPointShift);
                  ++idx0;
                  at(_newVector, i) = v;
                  at(tmp, idx1) = v;
               }
          }
        
        void TruspeechDecoderConverter::placePulses(int subframe)
          {
             using mods::utils::at;
             using mods::utils::arithmeticShifter::shiftRight;
             
             static constexpr int numberOfPulses = 7;
             std::array<s16, numberOfPulses> tmp {};
             
             auto& buf = at(_subframes, subframe);
             
             std::fill(buf.begin(), buf.end(), 0);
             for(size_t i=0; i<tmp.size(); ++i)
               {
                  auto& val = at(_pulseval, subframe);
                  auto t = static_cast<u32>(val) & 3U;
                  val = shiftRight(val, 2);
                  at(tmp, tmp.size()-1-i) = at(pulseScales, at(_pulseoff, subframe) * 4 + t);
               }
             
             static constexpr int coef1Shift = 15;
             int coef = shiftRight(at(_pulsepos, subframe), coef1Shift);
             size_t idxPulseValue = buf.size()/2;
             size_t idx2 = 0;
             for(size_t i=0, j=3; (i < buf.size()/2) && (j > 0); ++i)
               {
                  auto t = at(pulseValues, idxPulseValue);
                  ++idxPulseValue;
                  if(coef >= t)
                    {
                       coef -= t;
                    }
                  else
                    {
                       buf[i] = at(tmp, idx2);
                       ++idx2;
                       idxPulseValue += buf.size()/2;
                       --j;
                    }
               }
             static constexpr u16 coef2Mask = 0x7FFFU;
             coef = static_cast<u16>(at(_pulsepos, subframe)) & coef2Mask;
             idxPulseValue = 0;
             for(size_t i=buf.size()/2, j=4; (i < buf.size()) && (j > 0); ++i)
               {
                  auto t = at(pulseValues, idxPulseValue);
                  ++idxPulseValue;
                  if(coef >= t)
                    {
                       coef -= t;
                    }
                  else
                    {
                       buf[i] = at(tmp, idx2);
                       ++idx2;
                       idxPulseValue += buf.size()/2;
                       --j;
                    }
               }
          }
        
        void TruspeechDecoderConverter::updateFilters(int subframe)
          {
             using mods::utils::at;
             using mods::utils::arithmeticShifter::shiftRight;
             static constexpr int shiftOffset = _filterBufferLength - _subframeLength;
             
             auto& buf = at(_subframes, subframe);
             
             std::move(_filterBuffer.begin() + buf.size(), _filterBuffer.end(), _filterBuffer.begin());
             for(size_t i=0; i<buf.size(); ++i)
               {
                  at(_filterBuffer, i + shiftOffset) = buf[i] + at(_newVector,i) - shiftRight(at(_newVector,i), 3);
                  buf[i] += at(_newVector,i);
               }
          }
        
        void TruspeechDecoderConverter::synth(int subframe)
          {
             using mods::utils::at;
             using mods::utils::arithmeticShifter::shiftRight;
             
             static constexpr int roundConstant = 0x800;
             static constexpr u32 fixedPointShift = 12;
             static constexpr int clampLimit = 0x7FFE;
             
             auto& buf = at(_subframes, subframe);
             std::array<int, _numberOfFilterCoefficients> t {};
             
             auto* tmp = &_tmp1;
             auto filter = at(_filters, subframe);
             for(s16& v : buf)
               {
                  int sum = 0;
                  for(size_t k=0; k<tmp->size(); ++k)
                    {
                       sum += at(*tmp, k) * static_cast<s32>(at(filter, k));
                    }
                  sum = v + shiftRight(sum + roundConstant, fixedPointShift);
                  v = mods::utils::clamp(sum, -clampLimit, clampLimit);
                  for(size_t k=tmp->size()-1; k>0; --k)
                    {
                       at(*tmp, k) = at(*tmp, k-1);
                    }
                  at(*tmp, 0) = v;
               }
             
             static constexpr int decayShift = 15;
             for(size_t i=0; i<t.size(); ++i)
               {
                  at(t, i) = shiftRight(at(decay_35_64, i) * at(filter, i), decayShift);
               }
             
             tmp = &_tmp2;
             for(s16& v : buf)
               {
                  int sum = 0;
                  for(size_t k=0; k<tmp->size(); ++k)
                    {
                       sum += at(*tmp, k) * at(t, k);
                    }
                  for(size_t k=tmp->size()-1; k>0; --k)
                    {
                       at(*tmp, k) = at(*tmp, k-1);
                    }
                  at(*tmp, 0) = v;
                  v += shiftRight(-sum, fixedPointShift);
               }
             
             for(size_t i=0; i<t.size(); ++i)
               {
                  at(t, i) = shiftRight(at(decay_3_4, i) * at(filter, i), decayShift);
               }
             
             tmp = &_tmp3;
             for(s16& v : buf)
               {
                  s32 sum = v * static_cast<s16>(1U << fixedPointShift);
                  for(size_t k=0; k<tmp->size(); ++k)
                    {
                       sum += at(*tmp, k) * at(t, k);
                    }
                  for(size_t k=tmp->size()-1; k>0; --k)
                    {
                       at(*tmp, k) = at(*tmp, k-1);
                    }
                  at(*tmp, 0) = mods::utils::clamp(shiftRight(sum + roundConstant, fixedPointShift), -clampLimit, clampLimit);
                  
                  sum = shiftRight(at(*tmp, 1) * (_filtVal - shiftRight(_filtVal, 2)), 4) + sum;
                  sum = sum - shiftRight(sum, 3);
                  v = mods::utils::clamp(shiftRight(sum + roundConstant, fixedPointShift), -clampLimit, clampLimit);
               }
          }
        
     } // namespace wav
} // namespace mods