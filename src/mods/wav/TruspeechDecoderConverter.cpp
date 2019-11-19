
#include "mods/wav/TruspeechDecoderConverter.hpp"

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
             
             std::cout << "TODO: TruspeechDecoderConverter::decodeTruspeechFrame()" << std::endl;
             for(int i=0; i<32; ++i)
               {
                  _decodedBuffer[i] = _encodedBuffer[i];
               }
          }
        
        void TruspeechDecoderConverter::readParameters()
          {
             _lpc = _bitReader.read(24);
             _acl0 = _bitReader.read(7);
             _acl1 = _bitReader.read(2);
             _acl2 = _bitReader.read(7);
             _acl3 = _bitReader.read(2);
             _gain0 = _bitReader.read(12);
             _gain1 = _bitReader.read(12);
             _gain2 = _bitReader.read(12);
             _gain3 = _bitReader.read(12);
             _grid0 = _bitReader.read(1);
             _grid1 = _bitReader.read(1);
             _grid2 = _bitReader.read(1);
             _grid3 = _bitReader.read(1);
             _pos0 = _bitReader.read(12);
             _pos1 = _bitReader.read(12);
             _pos2 = _bitReader.read(12);
             _pos3 = _bitReader.read(12);
             _psig0 = _bitReader.read(4);
             _psig1 = _bitReader.read(4);
             _psig2 = _bitReader.read(4);
             _psig3 = _bitReader.read(4);
          }
     } // namespace wav
} // namespace mods