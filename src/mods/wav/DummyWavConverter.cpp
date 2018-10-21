
#include "mods/wav/DummyWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        DummyWavConverter::DummyWavConverter(WavConverter::ptr src)
          : _src(std::move(src))
            {
            }
        
        bool DummyWavConverter::isFinished() const
          {
             return _src->isFinished();
          }
        
        void DummyWavConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             _src->read(buf, len);
          }
     } //namespace wav
} // namespace mods
