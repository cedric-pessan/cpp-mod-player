
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
          _bitReader(_encodedBuffer)
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
             _vector[7] = _bitReader.read(3);
             _vector[6] = _bitReader.read(3);
             _vector[5] = _bitReader.read(3);
             _vector[4] = _bitReader.read(4);
             _vector[3] = _bitReader.read(4);
             _vector[2] = _bitReader.read(4);
             _vector[1] = _bitReader.read(5);
             _vector[0] = _bitReader.read(5);
             
             _flag = _bitReader.read(1) != 0;
             
             _offset1[0] = _bitReader.read(4) << 4;
             _offset2[3] = _bitReader.read(7);
             _offset2[2] = _bitReader.read(7);
             _offset2[1] = _bitReader.read(7);
             _offset2[0] = _bitReader.read(7);
             
             _offset1[1] = _bitReader.read(4);
             _pulseval[1] = _bitReader.read(14);
             _pulseval[0] = _bitReader.read(14);
             
             _offset1[1] |= _bitReader.read(4) << 4;
             _pulseval[3] = _bitReader.read(14);
             _pulseval[2] = _bitReader.read(14);
             
             _offset1[0] |= _bitReader.read(1);
             _pulsepos[0] = _bitReader.read(27);
             _pulseoff[0] = _bitReader.read(4);
             
             _offset1[0] |= _bitReader.read(1) << 1;
             _pulsepos[1] = _bitReader.read(27);
             _pulseoff[1] = _bitReader.read(4);
             
             _offset1[0] |= _bitReader.read(1) << 2;
             _pulsepos[2] = _bitReader.read(27);
             _pulseoff[2] = _bitReader.read(4);
             
             _offset1[0] |= _bitReader.read(1) << 3;
             _pulsepos[3] = _bitReader.read(27);
             _pulseoff[3] = _bitReader.read(4);
          }
        
        void TruspeechDecoderConverter::correlateFilter()
          {
             std::cout << "TODO: TruspeechDecoderConverter::correlateFilter()" << std::endl;
          }
     } // namespace wav
} // namespace mods