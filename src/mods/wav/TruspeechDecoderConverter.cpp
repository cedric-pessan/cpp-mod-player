
#include "mods/wav/TruspeechDecoderConverter.hpp"

namespace mods
{
   namespace wav
     {
        TruspeechDecoderConverter::TruspeechDecoderConverter(WavConverter::ptr src)
          : _src(std::move(src)),
          _destinationBuffer(allocateNewTempBuffer(0)),
          _decodedBuffer(_destinationBuffer.slice<u8>(0,0)),
          _itDecodedBuffer(_decodedBuffer.RBuffer<u8>::end()),
          _encodedArray {},
          _encodedBuffer(initializeArrayRWBuffer(_encodedArray)),
          _bitReader(_encodedBuffer)
            {
            }
        
        mods::utils::RWBuffer<u8> TruspeechDecoderConverter::allocateNewTempBuffer(size_t len)
          {
             _tempVec.resize(len);
             u8* ptr = _tempVec.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
             return mods::utils::RWBuffer<u8>(buffer);
          }
        
        template<typename ARRAY>
          mods::utils::RWBuffer<typename ARRAY::value_type> TruspeechDecoderConverter::initializeArrayRWBuffer(ARRAY& backArray)
            {
               auto* ptr = static_cast<u8*>(static_cast<void*>(backArray.data()));
               auto len = backArray.size();
               auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
               auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, len * sizeof(typename ARRAY::value_type), std::move(deleter));
               std::cout << "this is where interesting stuffs happen" << std::endl;
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
             _src->read(&_encodedBuffer, TRUSPEECH_FRAME_HEADER_SIZE);
             _bitReader.reset();
             
             _bitRate = _bitReader.read(1) != 0 ? BitRate::Low : BitRate::High;
             _vad = _bitReader.read(1) != 0 ? VoiceActive::No : VoiceActive::Yes;
             
             _vad = VoiceActive::Yes;
             _bitRate = BitRate::Low;
             
             if(_vad == VoiceActive::No)
               {
                  std::cout << "TODO: TruspeechDecoderConverter::decodeTruspeechFrame() vad" << std::endl;
               }
             else
               {
                  if(_bitRate == BitRate::High)
                    std::cout << "TODO: TruspeechDecoderConverter::decodeTruspeechFrame() not vad, high bitrate" << std::endl;
                  else
                    {
                       std::cout << "before slicing subframe" << std::endl;
                       auto subframe = _encodedBuffer.slice<u8>(1, LOW_BITRATE_FRAME_SIZE-1);
                       _src->read(&subframe, LOW_BITRATE_FRAME_SIZE-1);
                       
                       std::cout << "TODO: TruspeechDecoderConverter::decodeTruspeechFrame() not vad, low bitrate" << std::endl;
                    }
               }
             
             std::cout << "TODO: TruspeechDecoderConverter::decodeTruspeechFrame()" << std::endl;
          }
     } // namespace wav
} // namespace mods