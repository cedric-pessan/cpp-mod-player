
#include "mods/wav/GSMDecoderConverter.hpp"

namespace mods
{
   namespace wav
     {
        constexpr std::array<int, 8> GSMDecoderConverter::_LAR_SIZES;
        
        GSMDecoderConverter::GSMDecoderConverter(WavConverter::ptr src)
          : _src(std::move(src)),
          _decodedBuffer(allocateNewBuffer(_decodedVec, GSM_DECODED_FRAME_SIZE)),
          _itDecodedBuffer(_decodedBuffer.RBuffer<u8>::end()),
          _encodedBuffer(allocateNewBuffer(_encodedVec, GSM_ENCODED_PACK_SIZE)),
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
        
        mods::utils::RWBuffer<u8> GSMDecoderConverter::allocateNewBuffer(std::vector<u8>& backVec, size_t len)
          {
             backVec.resize(len);
             u8* ptr = backVec.data();
             auto deleter = std::make_unique<mods::utils::BufferBackend::EmptyDeleter>();
             auto buffer = std::make_shared<mods::utils::BufferBackend>(ptr, len, std::move(deleter));
             return mods::utils::RWBuffer<u8>(buffer);
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
             
             std::cout << "GSMDecoderConverter::uncompressGSMFrame()" << std::endl;
          }
        
        void GSMDecoderConverter::readParameters()
          {
             for(int i=0; i<8; ++i)
               {
                  _LAR[i] = _bitReader.read(_LAR_SIZES[i]);
               }
             
             std::cout << "GSMDecoderConverter::readParameters()" << std::endl;
          }
        
     } // namespace wav
} // namespace mods
