
#include "mods/converters/Converter.hpp"
#include "mods/utils/RBuffer.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/RWBufferBackend.hpp"
#include "mods/utils/arithmeticShifter.hpp"
#include "mods/utils/types.hpp"
#include "mods/wav/TruspeechDecoderConverter.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>

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
             _decodedBuffer.slice<s16>(0,                     _subframeLength),
             _decodedBuffer.slice<s16>(_subframeLength,       _subframeLength),
             _decodedBuffer.slice<s16>(_subframeLength * 2UL, _subframeLength),
             _decodedBuffer.slice<s16>(_subframeLength * 3UL, _subframeLength)
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
               auto len = backArray.size();
               auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
               auto buffer = std::make_unique<mods::utils::RWBufferBackend>(backArray.data(), len, std::move(deleter));
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
                       static_cast<s16>(0x8240), static_cast<s16>(0x8364), static_cast<s16>(0x84CE), static_cast<s16>(0x865D), static_cast<s16>(0x8805), static_cast<s16>(0x89DE), static_cast<s16>(0x8BD7), static_cast<s16>(0x8DF4),
                         static_cast<s16>(0x9051), static_cast<s16>(0x92E2), static_cast<s16>(0x95DE), static_cast<s16>(0x990F), static_cast<s16>(0x9C81), static_cast<s16>(0xA079), static_cast<s16>(0xA54C), static_cast<s16>(0xAAD2),
                         static_cast<s16>(0xB18A), static_cast<s16>(0xB90A), static_cast<s16>(0xC124), static_cast<s16>(0xC9CC), static_cast<s16>(0xD339), static_cast<s16>(0xDDD3), static_cast<s16>(0xE9D6), static_cast<s16>(0xF893),
                         static_cast<s16>(0x096F), static_cast<s16>(0x1ACA), static_cast<s16>(0x29EC), static_cast<s16>(0x381F), static_cast<s16>(0x45F9), static_cast<s16>(0x546A), static_cast<s16>(0x63C3), static_cast<s16>(0x73B5)
                    }
               };
             
             constexpr CodeBook<1,32> codeBook1
               {
                    {
                       static_cast<s16>(0x9F65), static_cast<s16>(0xB56B), static_cast<s16>(0xC583), static_cast<s16>(0xD371), static_cast<s16>(0xE018), static_cast<s16>(0xEBB4), static_cast<s16>(0xF61C), static_cast<s16>(0xFF59),
                         static_cast<s16>(0x085B), static_cast<s16>(0x1106), static_cast<s16>(0x1952), static_cast<s16>(0x214A), static_cast<s16>(0x28C9), static_cast<s16>(0x2FF8), static_cast<s16>(0x36E6), static_cast<s16>(0x3D92),
                         static_cast<s16>(0x43DF), static_cast<s16>(0x49BB), static_cast<s16>(0x4F46), static_cast<s16>(0x5467), static_cast<s16>(0x5930), static_cast<s16>(0x5DA3), static_cast<s16>(0x61EC), static_cast<s16>(0x65F9),
                         static_cast<s16>(0x69D4), static_cast<s16>(0x6D5A), static_cast<s16>(0x709E), static_cast<s16>(0x73AD), static_cast<s16>(0x766B), static_cast<s16>(0x78F0), static_cast<s16>(0x7B5A), static_cast<s16>(0x7DA5)
                    }
               };
             
             constexpr CodeBook<2,16> codeBook2
               {
                    {
                       static_cast<s16>(0x96F8), static_cast<s16>(0xA3B4), static_cast<s16>(0xAF45), static_cast<s16>(0xBA53), static_cast<s16>(0xC4B1), static_cast<s16>(0xCECC), static_cast<s16>(0xD86F), static_cast<s16>(0xE21E),
                         static_cast<s16>(0xEBF3), static_cast<s16>(0xF640), static_cast<s16>(0x00F7), static_cast<s16>(0x0C20), static_cast<s16>(0x1881), static_cast<s16>(0x269A), static_cast<s16>(0x376B), static_cast<s16>(0x4D60)
                    }
               };
             
             constexpr CodeBook<3,16> codeBook3
               {
                    {
                       static_cast<s16>(0xC654), static_cast<s16>(0xDEF2), static_cast<s16>(0xEFAA), static_cast<s16>(0xFD94), static_cast<s16>(0x096A), static_cast<s16>(0x143F), static_cast<s16>(0x1E7B), static_cast<s16>(0x282C),
                         static_cast<s16>(0x3176), static_cast<s16>(0x3A89), static_cast<s16>(0x439F), static_cast<s16>(0x4CA2), static_cast<s16>(0x557F), static_cast<s16>(0x5E50), static_cast<s16>(0x6718), static_cast<s16>(0x6F8D)
                    }
               };
             
             constexpr CodeBook<4,16> codeBook4
               {
                    {
                       static_cast<s16>(0xABE7), static_cast<s16>(0xBBA8), static_cast<s16>(0xC81C), static_cast<s16>(0xD326), static_cast<s16>(0xDD0E), static_cast<s16>(0xE5D4), static_cast<s16>(0xEE22), static_cast<s16>(0xF618),
                         static_cast<s16>(0xFE28), static_cast<s16>(0x064F), static_cast<s16>(0x0EB7), static_cast<s16>(0x17B8), static_cast<s16>(0x21AA), static_cast<s16>(0x2D8B), static_cast<s16>(0x3BA2), static_cast<s16>(0x4DF9)
                    }
               };
             
             constexpr CodeBook<5,8> codeBook5
               {
                    {
                       static_cast<s16>(0xD51B), static_cast<s16>(0xF12E), static_cast<s16>(0x042E), static_cast<s16>(0x13C7), static_cast<s16>(0x2260), static_cast<s16>(0x311B), static_cast<s16>(0x40DE), static_cast<s16>(0x5385)
                    }
               };
             
             constexpr CodeBook<6,8> codeBook6
               {
                  {
                     static_cast<s16>(0xB550), static_cast<s16>(0xC825), static_cast<s16>(0xD980), static_cast<s16>(0xE997), static_cast<s16>(0xF883), static_cast<s16>(0x0752), static_cast<s16>(0x1811), static_cast<s16>(0x2E18)
                  }
               };
             
             constexpr CodeBook<7,8> codeBook7
               {
                    {
                       static_cast<s16>(0xCEF0), static_cast<s16>(0xE4F9), static_cast<s16>(0xF6BB), static_cast<s16>(0x0646), static_cast<s16>(0x14F5), static_cast<s16>(0x23FF), static_cast<s16>(0x356F), static_cast<s16>(0x4A8D)
                    }
               };
             
             constexpr std::array<s16,8> decay_994_1000
               {
                  0x7F3B, 0x7E78, 0x7DB6, 0x7CF5, 0x7C35, 0x7B76, 0x7AB8, 0x79FC 
               };
             
             using Order2Coeffs = std::array<s16, 2>;
             constexpr std::array<Order2Coeffs, 25> order2Coeffs
               {{
                  { static_cast<s16>(0xED2F), static_cast<s16>(0x5239) },
                  { static_cast<s16>(0x54F1), static_cast<s16>(0xE4A9) },
                  { static_cast<s16>(0x2620), static_cast<s16>(0xEE3E) },
                  { static_cast<s16>(0x09D6), static_cast<s16>(0x2C40) },
                  { static_cast<s16>(0xEFB5), static_cast<s16>(0x2BE0) },
                  
                  { static_cast<s16>(0x3FE1), static_cast<s16>(0x3339) },
                  { static_cast<s16>(0x442F), static_cast<s16>(0xE6FE) },
                  { static_cast<s16>(0x4458), static_cast<s16>(0xF9DF) },
                  { static_cast<s16>(0xF231), static_cast<s16>(0x43DB) },
                  { static_cast<s16>(0x3DB0), static_cast<s16>(0xF705) },
                  
                  { static_cast<s16>(0x4F7B), static_cast<s16>(0xFEFB) },
                  { static_cast<s16>(0x26AD), static_cast<s16>(0x0CDC) },
                  { static_cast<s16>(0x33C2), static_cast<s16>(0x0739) },
                  { static_cast<s16>(0x12BE), static_cast<s16>(0x43A2) },
                  { static_cast<s16>(0x1BDF), static_cast<s16>(0x1F3E) },
                  
                  { static_cast<s16>(0x0211), static_cast<s16>(0x0796) },
                  { static_cast<s16>(0x2AEB), static_cast<s16>(0x163F) },
                  { static_cast<s16>(0x050D), static_cast<s16>(0x3A38) },
                  { static_cast<s16>(0x0D1E), static_cast<s16>(0x0D78) },
                  { static_cast<s16>(0x150F), static_cast<s16>(0x3346) },
                  
                  { static_cast<s16>(0x38A4), static_cast<s16>(0x0B7D) },
                  { static_cast<s16>(0x2D5D), static_cast<s16>(0x1FDF) },
                  { static_cast<s16>(0x19B7), static_cast<s16>(0x2822) },
                  { static_cast<s16>(0x0D99), static_cast<s16>(0x1F12) },
                  { static_cast<s16>(0x194C), static_cast<s16>(0x0CE6) }
               }};
             
             constexpr std::array<s16,64> pulseScales 
               {                  
                  static_cast<s16>(0x0002), static_cast<s16>(0x0006), static_cast<s16>(0xFFFE), static_cast<s16>(0xFFFA),
                  static_cast<s16>(0x0004), static_cast<s16>(0x000C), static_cast<s16>(0xFFFC), static_cast<s16>(0xFFF4),
                  static_cast<s16>(0x0006), static_cast<s16>(0x0012), static_cast<s16>(0xFFFA), static_cast<s16>(0xFFEE),
                  static_cast<s16>(0x000A), static_cast<s16>(0x001E), static_cast<s16>(0xFFF6), static_cast<s16>(0xFFE2),
                  static_cast<s16>(0x0010), static_cast<s16>(0x0030), static_cast<s16>(0xFFF0), static_cast<s16>(0xFFD0),
                  static_cast<s16>(0x0019), static_cast<s16>(0x004B), static_cast<s16>(0xFFE7), static_cast<s16>(0xFFB5),
                  static_cast<s16>(0x0028), static_cast<s16>(0x0078), static_cast<s16>(0xFFD8), static_cast<s16>(0xFF88),
                  static_cast<s16>(0x0040), static_cast<s16>(0x00C0), static_cast<s16>(0xFFC0), static_cast<s16>(0xFF40),
                  static_cast<s16>(0x0065), static_cast<s16>(0x012F), static_cast<s16>(0xFF9B), static_cast<s16>(0xFED1),
                  static_cast<s16>(0x00A1), static_cast<s16>(0x01E3), static_cast<s16>(0xFF5F), static_cast<s16>(0xFE1D),
                  static_cast<s16>(0x0100), static_cast<s16>(0x0300), static_cast<s16>(0xFF00), static_cast<s16>(0xFD00),
                  static_cast<s16>(0x0196), static_cast<s16>(0x04C2), static_cast<s16>(0xFE6A), static_cast<s16>(0xFB3E),
                  static_cast<s16>(0x0285), static_cast<s16>(0x078F), static_cast<s16>(0xFD7B), static_cast<s16>(0xF871),
                  static_cast<s16>(0x0400), static_cast<s16>(0x0C00), static_cast<s16>(0xFC00), static_cast<s16>(0xF400),
                  static_cast<s16>(0x0659), static_cast<s16>(0x130B), static_cast<s16>(0xF9A7), static_cast<s16>(0xECF5),
                  static_cast<s16>(0x0A14), static_cast<s16>(0x1E3C), static_cast<s16>(0xF5EC), static_cast<s16>(0xE1C4)
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
             at(_offset2,3) = static_cast<s32>(_bitReader.read(offset2Size));
             at(_offset2,2) = static_cast<s32>(_bitReader.read(offset2Size));
             at(_offset2,1) = static_cast<s32>(_bitReader.read(offset2Size));
             at(_offset2,0) = static_cast<s32>(_bitReader.read(offset2Size));
             
             u32 offset1_1 = _bitReader.read(4);
             at(_pulseval,1) = static_cast<s32>(_bitReader.read(pulsevalSize));
             at(_pulseval,0) = static_cast<s32>(_bitReader.read(pulsevalSize));
             
             offset1_1 |= _bitReader.read(4) << 4U;
             at(_pulseval,3) = static_cast<s32>(_bitReader.read(pulsevalSize));
             at(_pulseval,2) = static_cast<s32>(_bitReader.read(pulsevalSize));
             
             offset1_0 |= _bitReader.read(1);
             at(_pulsepos,0) = static_cast<s32>(_bitReader.read(pulseposSize));
             at(_pulseoff,0) = static_cast<s32>(_bitReader.read(4));
             
             offset1_0 |= _bitReader.read(1) << 1U;
             at(_pulsepos,1) = static_cast<s32>(_bitReader.read(pulseposSize));
             at(_pulseoff,1) = static_cast<s32>(_bitReader.read(4));
             
             offset1_0 |= _bitReader.read(1) << 2U;
             at(_pulsepos,2) = static_cast<s32>(_bitReader.read(pulseposSize));
             at(_pulseoff,2) = static_cast<s32>(_bitReader.read(4));
             
             offset1_0 |= _bitReader.read(1) << 3U;
             at(_pulsepos,3) = static_cast<s32>(_bitReader.read(pulseposSize));
             at(_pulseoff,3) = static_cast<s32>(_bitReader.read(4));
             
             at(_offset1,0) = static_cast<s32>(offset1_0);
             at(_offset1,1) = static_cast<s32>(offset1_1);
          }
        
        void TruspeechDecoderConverter::correlateFilter()
          {
             using mods::utils::at;
             using mods::utils::arithmeticShifter::shiftRight;
             using mods::utils::arithmeticShifter::Shift;
             
             std::array<s16,_numberOfFilterCoefficients> tmp {};
             auto& correlatedVector = at(_correlatedVectors, _currentCorrelatedVector);
             
             static constexpr s16 roundConstant = 0x4000;
             static constexpr int fixedPointShift = 15;
             
             for(size_t i=0; i<correlatedVector.size(); ++i)
               {
                  if(i > 0)
                    {
		       std::copy(correlatedVector.begin(), correlatedVector.begin() + i, tmp.begin());
                       for(size_t j=0; j<i; ++j)
                         {
                            at(correlatedVector,j) = static_cast<s16>(at(correlatedVector,j) + shiftRight((at(tmp, i - j -1) * at(_vector,i)) + roundConstant, static_cast<Shift>(fixedPointShift)));
                         }
                    }
                  static constexpr int offsetVector = 8;
                  at(correlatedVector,i) = static_cast<s16>(shiftRight(offsetVector - at(_vector,i), static_cast<Shift>(3)));
               }
             for(size_t i=0; i<correlatedVector.size(); ++i)
               {
                  at(correlatedVector,i) = static_cast<s16>(shiftRight(at(correlatedVector,i) * at(decay_994_1000,i), static_cast<Shift>(fixedPointShift)));
               }
             
             _filtVal = at(_vector,0);
          }
        
        void TruspeechDecoderConverter::filtersMerge()
          {
             using mods::utils::at;
             using mods::utils::arithmeticShifter::shiftRight;
             using mods::utils::arithmeticShifter::Shift;
             
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
                       at(filter1, i) = static_cast<s16>(shiftRight((at(correlatedVector, i) * mixCoef1) + (at(previousCorrelatedVector, i) * mixCoef2) + roundConstant, static_cast<Shift>(fixedPointShift)));
                       at(filter2, i) = static_cast<s16>(shiftRight((at(correlatedVector, i) * mixCoef2) + (at(previousCorrelatedVector, i) * mixCoef1) + roundConstant, static_cast<Shift>(fixedPointShift)));
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
             using mods::utils::arithmeticShifter::Shift;
             
             static constexpr int clearFilterSpecialCase = 127;
             static constexpr int offset2Factor = 25;
             static constexpr int baseOffset = 18;
             
             std::array<s16, _filterBufferLength + _subframeLength> tmp {};
             
             const int toff = at(_offset2, subframe);
             if(toff == clearFilterSpecialCase)
	       {
		  std::fill(_newVector.begin(), _newVector.end(), 0);
		  return;
	       }
             for(size_t i=0; i<_filterBuffer.size(); ++i)
               {
                  at(tmp,i) = static_cast<s16>(at(_filterBuffer,i));
               }
             s32 off = (toff / offset2Factor) + at(_offset1, shiftRight(subframe, static_cast<Shift>(1))) + baseOffset;
             off = mods::utils::clamp(off, 0, _filterBufferLength-1);
             size_t idx0 = _filterBufferLength - 1 - off;
             size_t idx1 = _filterBuffer.size();
             const auto& filter = at(order2Coeffs, toff % order2Coeffs.size());
             
             static constexpr int roundConstant = 0x2000;
             static constexpr int fixedPointShift = 14;
             for(size_t i=0; i<_newVector.size(); ++i,++idx1)
               {
                  const auto val = static_cast<s16>(shiftRight((at(tmp, idx0)   * at(filter,0)) + 
                                                               (at(tmp, idx0+1) * at(filter,1)) + roundConstant, static_cast<Shift>(fixedPointShift)));
                  ++idx0;
                  at(_newVector, i) = val;
                  at(tmp, idx1) = val;
               }
          }
        
        void TruspeechDecoderConverter::placePulses(int subframe)
          {
             using mods::utils::at;
             using mods::utils::arithmeticShifter::shiftRight;
             using mods::utils::arithmeticShifter::Shift;
             
             static constexpr int numberOfPulses = 7;
             std::array<s16, numberOfPulses> tmp {};
             
             auto& buf = at(_subframes, subframe);
             
             std::fill(buf.begin(), buf.end(), 0);
             for(size_t i=0; i<tmp.size(); ++i)
               {
                  auto& val = at(_pulseval, subframe);
                  auto offset = static_cast<u32>(val) & 3U;
                  val = shiftRight(val, static_cast<Shift>(2));
                  at(tmp, tmp.size()-1-i) = at(pulseScales, (at(_pulseoff, subframe) * 4) + offset);
               }
             
             static constexpr int coef1Shift = 15;
             int coef = shiftRight(at(_pulsepos, subframe), static_cast<Shift>(coef1Shift));
             size_t idxPulseValue = buf.size()/2;
             size_t idx2 = 0;
             for(size_t i=0, j=3; (i < buf.size()/2) && (j > 0); ++i)
               {
                  auto offset = at(pulseValues, idxPulseValue);
                  ++idxPulseValue;
                  if(coef >= offset)
                    {
                       coef -= offset;
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
                  auto offset = at(pulseValues, idxPulseValue);
                  ++idxPulseValue;
                  if(coef >= offset)
                    {
                       coef -= offset;
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
             using mods::utils::arithmeticShifter::Shift;
             static constexpr int shiftOffset = _filterBufferLength - _subframeLength;
             
             auto& buf = at(_subframes, subframe);
             
             std::move(_filterBuffer.begin() + buf.size(), _filterBuffer.end(), _filterBuffer.begin());
             for(size_t i=0; i<buf.size(); ++i)
               {
                  at(_filterBuffer, i + shiftOffset) = buf[i] + at(_newVector,i) - shiftRight(at(_newVector,i), static_cast<Shift>(3));
                  buf[i] = static_cast<s16>(buf[i] + at(_newVector,i));
               }
          }
        
        void TruspeechDecoderConverter::synth(int subframe)
          {
             using mods::utils::at;
             using mods::utils::arithmeticShifter::shiftRight;
             using mods::utils::arithmeticShifter::Shift;
             
             static constexpr int roundConstant = 0x800;
             static constexpr u32 fixedPointShift = 12;
             static constexpr int clampLimit = 0x7FFE;
             
             auto& buf = at(_subframes, subframe);
             std::array<int, _numberOfFilterCoefficients> tcoefs {};
             
             auto* tmp = &_tmp1;
             auto filter = at(_filters, subframe);
             for(s16& val : buf)
               {
                  int sum = 0;
                  for(size_t k=0; k<tmp->size(); ++k)
                    {
                       sum += at(*tmp, k) * static_cast<s32>(at(filter, k));
                    }
                  sum = val + shiftRight(sum + roundConstant, static_cast<Shift>(fixedPointShift));
                  val = static_cast<s16>(mods::utils::clamp(sum, -clampLimit, clampLimit));
                  for(size_t k=tmp->size()-1; k>0; --k)
                    {
                       at(*tmp, k) = at(*tmp, k-1);
                    }
                  at(*tmp, 0) = val;
               }
             
             static constexpr int decayShift = 15;
             for(size_t i=0; i<tcoefs.size(); ++i)
               {
                  at(tcoefs, i) = shiftRight(at(decay_35_64, i) * at(filter, i), static_cast<Shift>(decayShift));
               }
             
             tmp = &_tmp2;
             for(s16& val : buf)
               {
                  int sum = 0;
                  for(size_t k=0; k<tmp->size(); ++k)
                    {
                       sum += at(*tmp, k) * at(tcoefs, k);
                    }
                  for(size_t k=tmp->size()-1; k>0; --k)
                    {
                       at(*tmp, k) = at(*tmp, k-1);
                    }
                  at(*tmp, 0) = val;
                  val = static_cast<s16>(val + shiftRight(-sum, static_cast<Shift>(fixedPointShift)));
               }
             
             for(size_t i=0; i<tcoefs.size(); ++i)
               {
                  at(tcoefs, i) = shiftRight(at(decay_3_4, i) * at(filter, i), static_cast<Shift>(decayShift));
               }
             
             tmp = &_tmp3;
             for(s16& val : buf)
               {
                  s32 sum = val * static_cast<s16>(1U << fixedPointShift);
                  for(size_t k=0; k<tmp->size(); ++k)
                    {
                       sum += at(*tmp, k) * at(tcoefs, k);
                    }
                  for(size_t k=tmp->size()-1; k>0; --k)
                    {
                       at(*tmp, k) = at(*tmp, k-1);
                    }
                  at(*tmp, 0) = static_cast<s16>(mods::utils::clamp(shiftRight(sum + roundConstant, static_cast<Shift>(fixedPointShift)), -clampLimit, clampLimit));
                  
                  sum = shiftRight(at(*tmp, 1) * (_filtVal - shiftRight(_filtVal, static_cast<Shift>(2))), static_cast<Shift>(4)) + sum;
                  sum = sum - shiftRight(sum, static_cast<Shift>(3));
                  val = static_cast<s16>(mods::utils::clamp(shiftRight(sum + roundConstant, static_cast<Shift>(fixedPointShift)), -clampLimit, clampLimit));
               }
          }
        
     } // namespace wav
} // namespace mods