
#include "mods/wav/BytesSkipConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        template<int KEEP, int SKIP>
          BytesSkipConverter<KEEP, SKIP>::BytesSkipConverter(WavConverter::ptr src)
            : _src(std::move(src)),
          _temp(allocateTempBuffer(0))
          {
          }
        
        template<int KEEP, int SKIP>
          bool BytesSkipConverter<KEEP, SKIP>::isFinished() const
          {
             return _src->isFinished();
          }
        
        template<int KEEP, int SKIP>
          void BytesSkipConverter<KEEP, SKIP>::read(mods::utils::RWBuffer<u8>* buf, int len)
            {
               if((len % KEEP) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in BytesSkipWavConverter" << std::endl;
                 }
               
               int nbElems = len / KEEP;
               int toReadLen = nbElems * (KEEP + SKIP);
               
               allocateTempBuffer(toReadLen);
               
               _src->read(&_temp, toReadLen);
               
               for(int i=0; i<nbElems; ++i)
                 {
                    for(int j=0; j<KEEP; ++j)
                      {
                         u8 value = _temp[i*(SKIP+KEEP) + j + SKIP];
                         (*buf)[i*KEEP + j] = value;
                      }
                 }
            }
        
        template<int KEEP, int SKIP>
          mods::utils::RWBuffer<u8> BytesSkipConverter<KEEP, SKIP>::allocateTempBuffer(size_t len)
            {
               if(_temp.size() < len)
                 {
                    _tempVec.resize(len);
                    u8* ptr = _tempVec.data();
                    auto deleter = std::make_unique<mods::utils::BufferBackend::EmptyDeleter>();
                    auto buffer = std::make_shared<mods::utils::BufferBackend>(ptr, len, std::move(deleter));
                    _temp = mods::utils::RWBuffer<u8>(buffer);
                 }
               return _temp;
            }
        
        template class BytesSkipConverter<2, 1>;
     } // namespace wav
} // namespace mods
