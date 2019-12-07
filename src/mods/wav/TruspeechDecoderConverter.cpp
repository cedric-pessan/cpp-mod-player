
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
          _encodedArray {},
          _encodedBuffer(initializeArrayRWBuffer(_encodedArray)),
          _bitReader(_encodedBuffer),
          _vector {},
          _offset1 {},
          _offset2 {},
          _pulseval {},
          _pulsepos {},
          _pulseoff {},
          _correlatedVector {}
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
             
             std::cout << "TODO: TruspeechDecoderConverter::decodeTruspeechFrame()" << std::endl;
             for(int i=0; i<32; ++i)
               {
                  _decodedBuffer[i] = _encodedBuffer[i];
               }
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
             
             for(int i=0; i<8; ++i)
               {
                  if(i > 0)
                    {
                       std::copy(tmp.begin(), tmp.begin() + i, _correlatedVector.begin());
                       for(int j=0; j<i; ++j)
                         {
                            at(_correlatedVector,j) += (at(tmp, i - j -1) * at(_vector,i) + 0x4000) >> 15;
                         }
                    }
                  at(_correlatedVector,i) = (8 - at(_vector,i)) >> 3;
               }
             for(int i=0; i<8; ++i)
               {
                  at(_correlatedVector,i) = (at(_correlatedVector,i) * at(decay_994_1000,i)) >> 15;
               }
             
             _filtVal = at(_vector,0);
          }
        
        void TruspeechDecoderConverter::filtersMerge()
          {
             std::cout << "TruspeechDecoderConverter::filtersMerge()" << std::endl;
          }
        
     } // namespace wav
} // namespace mods