
#include "mods/wav/ResampleConverter.hpp"

namespace mods
{
   namespace wav
     {
        ResampleConverter::ResampleConverter(WavConverter::ptr src)
          : _src(std::move(src))
            {
            }
        
        bool ResampleConverter::isFinished() const
          {
             return _src->isFinished();
          }
        
        void ResampleConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             _src->read(buf, len);
             std::cout << "TODO: ResampleConverter::read(mods::utils::RWBuffer<u8>*, int)" << std::endl;
          }
     } // namespace wav
} // namespace mods