
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
             std::cout << "TODO: ResampleConverter::isFinished() const" << std::endl;
             return false;
          }
        
        void ResampleConverter::read(mods::utils::RWBuffer<u8>* buf, int len)
          {
             _src->read(buf, len);
             std::cout << "TODO: ResampleConverter::read(mods::utils::RWBuffer<u8>*, int)" << std::endl;
          }
     } // namespace wav
} // namespace mods