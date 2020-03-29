
#include "mods/wav/TruspeechDecoderConverter.hpp"

/*
 * based on ffmpeg source algorithm
 */

namespace mods
{
   namespace wav
     {
        TruspeechDecoderConverter::TruspeechDecoderConverter(WavConverter::ptr src)
          : _src(std::move(src)),
          _decodedArray {},
          _decodedBuffer(initializeArrayRWBuffer(_decodedArray)),
          _itDecodedBuffer(_decodedBuffer.RBuffer<u8>::end()),
          _subframes
          {
             _decodedBuffer.slice<s16>(0,    60),
             _decodedBuffer.slice<s16>(60*2, 60),
             _decodedBuffer.slice<s16>(120*2,60),
             _decodedBuffer.slice<s16>(180*2,60)
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
          _filters {},
          _filterBuffer {},
          _newVector {},
          _tmp1 {},
          _tmp2 {},
          _tmp3 {}
          {
          }
        
        template<typename ARRAY>
          mods::utils::RWBuffer<typename ARRAY::value_type> TruspeechDecoderConverter::initializeArrayRWBuffer(ARRAY& backArray)
            {
               auto* ptr = static_cast<u8*>(static_cast<void*>(backArray.data()));
               auto len = backArray.size();
               auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
               auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, len * sizeof(typename ARRAY::value_type), std::move(deleter));
               return mods::utils::RWBuffer<u8>(buffer).slice<typename ARRAY::value_type>(0, len);
            }
        
        bool TruspeechDecoderConverter::isFinished() const
          {
             return _itDecodedBuffer == _decodedBuffer.RBuffer<u8>::end() && _src->isFinished();
          }
        
        void TruspeechDecoderConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
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
                            decodeTruspeechFrame();
                            _itDecodedBuffer = _decodedBuffer.RBuffer<u8>::begin();
                         }
                    }
               }
          }
        
        void TruspeechDecoderConverter::decodeTruspeechFrame()
          {
             _src->read(&_encodedBuffer, TRUSPEECH_ENCODED_FRAME_SIZE);
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
             constexpr std::array<s16,32> codeBook0
               {
                  s16(0x8240), s16(0x8364), s16(0x84CE), s16(0x865D), s16(0x8805), s16(0x89DE), s16(0x8BD7), s16(0x8DF4),
                    s16(0x9051), s16(0x92E2), s16(0x95DE), s16(0x990F), s16(0x9C81), s16(0xA079), s16(0xA54C), s16(0xAAD2),
                    s16(0xB18A), s16(0xB90A), s16(0xC124), s16(0xC9CC), s16(0xD339), s16(0xDDD3), s16(0xE9D6), s16(0xF893),
                    s16(0x096F), s16(0x1ACA), s16(0x29EC), s16(0x381F), s16(0x45F9), s16(0x546A), s16(0x63C3), s16(0x73B5)
               };
             
             constexpr std::array<s16,32> codeBook1
               {
                  s16(0x9F65), s16(0xB56B), s16(0xC583), s16(0xD371), s16(0xE018), s16(0xEBB4), s16(0xF61C), s16(0xFF59),
                    s16(0x085B), s16(0x1106), s16(0x1952), s16(0x214A), s16(0x28C9), s16(0x2FF8), s16(0x36E6), s16(0x3D92),
                    s16(0x43DF), s16(0x49BB), s16(0x4F46), s16(0x5467), s16(0x5930), s16(0x5DA3), s16(0x61EC), s16(0x65F9),
                    s16(0x69D4), s16(0x6D5A), s16(0x709E), s16(0x73AD), s16(0x766B), s16(0x78F0), s16(0x7B5A), s16(0x7DA5)
               };
             
             constexpr std::array<s16,16> codeBook2
               {
                  s16(0x96F8), s16(0xA3B4), s16(0xAF45), s16(0xBA53), s16(0xC4B1), s16(0xCECC), s16(0xD86F), s16(0xE21E),
                    s16(0xEBF3), s16(0xF640), s16(0x00F7), s16(0x0C20), s16(0x1881), s16(0x269A), s16(0x376B), s16(0x4D60)
               };
             
             constexpr std::array<s16,16> codeBook3
               {
                  s16(0xC654), s16(0xDEF2), s16(0xEFAA), s16(0xFD94), s16(0x096A), s16(0x143F), s16(0x1E7B), s16(0x282C),
                    s16(0x3176), s16(0x3A89), s16(0x439F), s16(0x4CA2), s16(0x557F), s16(0x5E50), s16(0x6718), s16(0x6F8D)
               };
             
             constexpr std::array<s16,16> codeBook4
               {
                  s16(0xABE7), s16(0xBBA8), s16(0xC81C), s16(0xD326), s16(0xDD0E), s16(0xE5D4), s16(0xEE22), s16(0xF618),
                    s16(0xFE28), s16(0x064F), s16(0x0EB7), s16(0x17B8), s16(0x21AA), s16(0x2D8B), s16(0x3BA2), s16(0x4DF9)
               };
             
             constexpr std::array<s16,8> codeBook5
               {
                  s16(0xD51B), s16(0xF12E), s16(0x042E), s16(0x13C7), s16(0x2260), s16(0x311B), s16(0x40DE), s16(0x5385)
               };
             
             constexpr std::array<s16,8> codeBook6
               {
                  s16(0xB550), s16(0xC825), s16(0xD980), s16(0xE997), s16(0xF883), s16(0x0752), s16(0x1811), s16(0x2E18)
               };
             
             constexpr std::array<s16,8> codeBook7
               {
                  s16(0xCEF0), s16(0xE4F9), s16(0xF6BB), s16(0x0646), s16(0x14F5), s16(0x23FF), s16(0x356F), s16(0x4A8D)
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
             
             at(_vector,7) = at(codeBook7, _bitReader.read(3));
             at(_vector,6) = at(codeBook6, _bitReader.read(3));
             at(_vector,5) = at(codeBook5, _bitReader.read(3));
             at(_vector,4) = at(codeBook4, _bitReader.read(4));
             at(_vector,3) = at(codeBook3, _bitReader.read(4));
             at(_vector,2) = at(codeBook2, _bitReader.read(4));
             at(_vector,1) = at(codeBook1, _bitReader.read(5));
             at(_vector,0) = at(codeBook0, _bitReader.read(5));
             
             _flag = _bitReader.read(1) != 0;
             
             at(_offset1,0) = _bitReader.read(4) << 4;
             at(_offset2,3) = _bitReader.read(7);
             at(_offset2,2) = _bitReader.read(7);
             at(_offset2,1) = _bitReader.read(7);
             at(_offset2,0) = _bitReader.read(7);
             
             at(_offset1,1) = _bitReader.read(4);
             at(_pulseval,1) = _bitReader.read(14);
             at(_pulseval,0) = _bitReader.read(14);
             
             at(_offset1,1) |= _bitReader.read(4) << 4;
             at(_pulseval,3) = _bitReader.read(14);
             at(_pulseval,2) = _bitReader.read(14);
             
             at(_offset1,0) |= _bitReader.read(1);
             at(_pulsepos,0) = _bitReader.read(27);
             at(_pulseoff,0) = _bitReader.read(4);
             
             at(_offset1,0) |= _bitReader.read(1) << 1;
             at(_pulsepos,1) = _bitReader.read(27);
             at(_pulseoff,1) = _bitReader.read(4);
             
             at(_offset1,0) |= _bitReader.read(1) << 2;
             at(_pulsepos,2) = _bitReader.read(27);
             at(_pulseoff,2) = _bitReader.read(4);
             
             at(_offset1,0) |= _bitReader.read(1) << 3;
             at(_pulsepos,3) = _bitReader.read(27);
             at(_pulseoff,3) = _bitReader.read(4);
          }
        
        void TruspeechDecoderConverter::correlateFilter()
          {
             using mods::utils::at;
             
             std::array<s16,8> tmp;
             auto& correlatedVector = at(_correlatedVectors, _currentCorrelatedVector);
             
             for(int i=0; i<8; ++i)
               {
                  if(i > 0)
                    {
		       std::copy(correlatedVector.begin(), correlatedVector.begin() + i, tmp.begin());
                       for(int j=0; j<i; ++j)
                         {
                            at(correlatedVector,j) += (at(tmp, i - j -1) * at(_vector,i) + 0x4000) >> 15;
                         }
                    }
                  at(correlatedVector,i) = (8 - at(_vector,i)) >> 3;
               }
             for(int i=0; i<8; ++i)
               {
                  at(correlatedVector,i) = (at(correlatedVector,i) * at(decay_994_1000,i)) >> 15;
               }
             
             _filtVal = at(_vector,0);
          }
        
        void TruspeechDecoderConverter::filtersMerge()
          {
             using mods::utils::at;
             
             auto& previousCorrelatedVector = at(_correlatedVectors, 1 - _currentCorrelatedVector);
             auto& correlatedVector = at(_correlatedVectors, _currentCorrelatedVector);
             
             if(!_flag)
               {
                  for(int i=0; i<8; ++i)
                    {
                       at(_filters, i)   = at(previousCorrelatedVector, i);
                       at(_filters, i+8) = at(previousCorrelatedVector, i);
                    }
               }
             else
               {
                  for(int i=0; i<8; ++i)
                    {
                       at(_filters, i) = (at(correlatedVector, i) * 21846 + at(previousCorrelatedVector, i) * 10923 + 16384) >> 15;
                       at(_filters, i+8) = (at(correlatedVector, i) * 10923 + at(previousCorrelatedVector, i) * 21846 + 16384) >> 15;
                    }
               }
             
             for(int i=0; i<8; ++i)
               {
                  at(_filters, i+16) = at(correlatedVector, i);
                  at(_filters, i+24) = at(correlatedVector, i);
               }
          }
        
        void TruspeechDecoderConverter::applyTwoPointFilter(int subframe)
          {
             using mods::utils::at;
             
             std::array<s16, 146 + 60> tmp;
             
             int t = at(_offset2, subframe);
             if(t == 127)
	       {
		  std::fill(_newVector.begin(), _newVector.end(), 0);
		  return;
	       }
             for(int i=0; i<146; ++i)
               {
                  at(tmp,i) = at(_filterBuffer,i);
               }
             s32 off = (t / 25) + at(_offset1, subframe >> 1) + 18;
             off = mods::utils::clamp(off, 0, 145);
             auto it0 = tmp.begin() + (145 - off);
             auto it1 = tmp.begin() + 146;
             auto& filter = at(order2Coeffs, t % 25);
             for(int i=0; i<60; ++i,++it1)
               {
                  t = (*it0 * at(filter,0) + *(it0+1) * at(filter,1) + 0x2000) >> 14;
                  ++it0;
                  at(_newVector, i) = t;
                  *it1 = t;
               }
          }
        
        void TruspeechDecoderConverter::placePulses(int subframe)
          {
             using mods::utils::at;
             std::array<s16, 7> tmp;
             
             auto& buf = at(_subframes, subframe);
             
             std::fill(buf.begin(), buf.end(), 0);
             for(int i=0; i<7; ++i)
               {
                  auto t = at(_pulseval, subframe) & 3;
                  at(_pulseval, subframe) >>= 2;
                  at(tmp, 6-i) = at(pulseScales, at(_pulseoff, subframe) * 4 + t);
               }
             
             int coef = at(_pulsepos, subframe) >> 15;
             auto it1 = pulseValues.begin() + 30;
             auto it2 = tmp.begin();
             for(int i=0, j=3; (i < 30) && (j > 0); ++i)
               {
                  auto t = *it1;
                  ++it1;
                  if(coef >= t)
                    {
                       coef -= t;
                    }
                  else
                    {
                       buf[i] = *it2;
                       ++it2;
                       it1 += 30;
                       --j;
                    }
               }
             coef = at(_pulsepos, subframe) & 0x7FFF;
             it1 = pulseValues.begin();
             for(int i=30, j=4; (i < 60) && (j > 0); ++i)
               {
                  auto t = *it1;
                  ++it1;
                  if(coef >= t)
                    {
                       coef -= t;
                    }
                  else
                    {
                       buf[i] = *it2;
                       ++it2;
                       it1 += 30;
                       --j;
                    }
               }
          }
        
        void TruspeechDecoderConverter::updateFilters(int subframe)
          {
             using mods::utils::at;
             
             auto& buf = at(_subframes, subframe);
             
             std::move(_filterBuffer.begin() + 60, _filterBuffer.end(), _filterBuffer.begin());
             for(int i=0; i<60; ++i)
               {
                  at(_filterBuffer, i + 86) = buf[i] + at(_newVector,i) - (at(_newVector,i) >> 3);
                  buf[i] += at(_newVector,i);
               }
          }
        
        void TruspeechDecoderConverter::synth(int subframe)
          {
             using mods::utils::at;
             
             auto& buf = at(_subframes, subframe);
             std::array<int, 8> t;
             
             auto* tmp = &_tmp1;
             auto it = _filters.begin() + subframe * 8;
             for(int i=0; i<60; ++i)
               {
                  int sum = 0;
                  for(int k=0; k<8; ++k)
                    {
                       sum += at(*tmp, k) * (u32)*(it + k);
                    }
                  sum = buf[i] + ((sum + 0x800) >> 12);
                  buf[i] = mods::utils::clamp(sum, -0x7FFE, 0x7FFE);
                  for(int k=7; k>0; --k)
                    {
                       at(*tmp, k) = at(*tmp, k-1);
                    }
                  at(*tmp, 0) = buf[i];
               }
             
             for(int i=0; i<8; ++i)
               {
                  at(t, i) = (at(decay_35_64, i) * *(it + i)) >> 15;
               }
             
             tmp = &_tmp2;
             for(int i=0; i<60; ++i)
               {
                  int sum = 0;
                  for(int k=0; k<8; ++k)
                    {
                       sum += at(*tmp, k) * at(t, k);
                    }
                  for(int k=7; k>0; --k)
                    {
                       at(*tmp, k) = at(*tmp, k-1);
                    }
                  at(*tmp, 0) = buf[i];
                  buf[i] += (-sum) >> 12;
               }
             
             for(int i=0; i<8; ++i)
               {
                  at(t, i) = (at(decay_3_4, i) * *(it + i)) >> 15;
               }
             
             tmp = &_tmp3;
             for(int i=0; i<60; ++i)
               {
                  int sum = buf[i] * (1 << 12);
                  for(int k=0; k<8; ++k)
                    {
                       sum += at(*tmp, k) * at(t, k);
                    }
                  for(int k=7; k>0; --k)
                    {
                       at(*tmp, k) = at(*tmp, k-1);
                    }
                  at(*tmp, 0) = mods::utils::clamp((sum + 0x800) >> 12, -0x7FFE, 0x7FFE);
                  
                  sum = ((at(*tmp, 1) * (_filtVal - (_filtVal >> 2))) >> 4) + sum;
                  sum = sum - (sum >> 3);
                  buf[i] = mods::utils::clamp((sum + 0x800) >> 12, -0x7FFE, 0x7FFE);
               }
          }
        
     } // namespace wav
} // namespace mods