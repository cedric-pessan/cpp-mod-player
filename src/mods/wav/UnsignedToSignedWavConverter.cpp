
#include "mods/wav/UnsignedToSignedWavConverter.hpp"

namespace mods
{
   namespace wav
     {
        UnsignedToSignedWavConverter::UnsignedToSignedWavConverter(WavConverter::ptr src)
          : _src(std::move(src))
            {
            }
        
        bool UnsignedToSignedWavConverter::isFinished() const
          {
             return _src->isFinished();
          }
        
        void UnsignedToSignedWavConverter::read(mods::utils::RWBuffer<u8>& buf, int len)
          {
             _src->read(buf, len);
             std::cout << "TODO: UnsignedToSignedWavConverter::read(...)" << std::endl;
          }
     } // namespace wav
} // namespace mods
