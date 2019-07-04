
#include "mods/wav/GSMDecoderConverter.hpp"

namespace mods
{
   namespace wav
     {
        GSMDecoderConverter::GSMDecoderConverter(WavConverter::ptr src)
          : _src(std::move(src)),
          _decodedBuffer(allocateNewBuffer(_decodedVec, GSM_FRAME_SIZE * 2)),
          _itDecodedBuffer(_decodedBuffer.RBuffer<u8>::end())
            {
            }
        
        bool GSMDecoderConverter::isFinished() const
          {
             return _itDecodedBuffer == _decodedBuffer.RBuffer<u8>::end() && _src->isFinished();
          }
        
        void GSMDecoderConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             _src->read(buf, len);
             std::cout << "TODO: GSMDecoderConverter::read(mods::utils::RWBuffer<u8>*, int)" << std::endl;
          }
        
        mods::utils::RWBuffer<u8> GSMDecoderConverter::allocateNewBuffer(std::vector<u8>& backVec, size_t len)
          {
             backVec.resize(len);
             u8* ptr = backVec.data();
             auto deleter = std::make_unique<mods::utils::BufferBackend::EmptyDeleter>();
             auto buffer = std::make_shared<mods::utils::BufferBackend>(ptr, len, std::move(deleter));
             return mods::utils::RWBuffer<u8>(buffer);
          }
     } // namespace wav
} // namespace mods