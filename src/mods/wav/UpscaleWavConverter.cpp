
#include "mods/wav/UpscaleWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        UpscaleWavConverter::UpscaleWavConverter(WavConverter::ptr src)
          : _src(std::move(src))
            {
            }
        
        bool UpscaleWavConverter::isFinished() const
          {
             return _src->isFinished();
          }
        
        void UpscaleWavConverter::read(mods::utils::RWBuffer<u8>& buf, int len)
          {
             _src->read(buf, len);
             std::cout << "TODO: UpscaleWavConverter::read(...)" << std::endl;
          }
     } // namespace wav
} // namespace mods
