
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
             if((len % sizeof(double)) != 0)
               {
                  std::cout << "TODO: wrong buffer length in ResampleConverter" << std::endl;
               }
             
             //_src->read(buf, len);
             
             int nbElems = len / sizeof(double);
             
             auto outView = buf->slice<double>(0, nbElems);
             
             for(int i=0; i<nbElems; ++i)
               {
                  outView[i] = getNextDecimatedSample();
               }
             
             std::cout << "TODO: ResampleConverter::read(mods::utils::RWBuffer<u8>*, int)" << std::endl;
          }
        
        double ResampleConverter::getNextDecimatedSample()
          {
             std::cout << "TODO: ResampleConverter::getNextDecimatedSample()" << std::endl;
             return 0.0;
          }
     } // namespace wav
} // namespace mods