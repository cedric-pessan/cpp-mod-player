
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
             
             std::cout << "TODO: TruspeechDecoderConverter::decodeTruspeechFrame()" << std::endl;
             for(int i=0; i<32; ++i)
               {
                  _decodedBuffer[i] = _encodedBuffer[i];
               }
          }
        
        void TruspeechDecoderConverter::readParameters()
          {
             using mods::utils::at;
             
             at(_vector,7) = _bitReader.read(3);
             at(_vector,6) = _bitReader.read(3);
             at(_vector,5) = _bitReader.read(3);
             at(_vector,4) = _bitReader.read(4);
             at(_vector,3) = _bitReader.read(4);
             at(_vector,2) = _bitReader.read(4);
             at(_vector,1) = _bitReader.read(5);
             at(_vector,0) = _bitReader.read(5);
             
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
        
        namespace
          {
             constexpr std::array<s16,8> ts_decay_994_1000
               {
                  0x7F3B, 0x7E78, 0x7DB6, 0x7CF5, 0x7C35, 0x7B76, 0x7AB8, 0x79FC 
               };
          } // namespace
        
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
                  at(_correlatedVector,i) = (at(_correlatedVector,i) * at(ts_decay_994_1000,i)) >> 15;
               }
             
             _filtVal = at(_vector,0);
          }
     } // namespace wav
} // namespace mods