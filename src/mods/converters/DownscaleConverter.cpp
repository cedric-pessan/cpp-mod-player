
#include "mods/converters/DownscaleConverter.hpp"

#include <iostream>

namespace mods
{
   namespace converters
     {
        template<typename TOut, typename TIn>
          DownscaleConverter<TOut, TIn>::DownscaleConverter(Converter::ptr src)
            : _src(std::move(src)),
          _temp(allocateNewTempBuffer(0))
              {
                 if(sizeof(TOut) > sizeof(TIn))
                   {
                      std::cout << "TOut should be smaller than TIn in DownscaleConverter" << std::endl;
                   }
              }
        
        template<typename TOut, typename TIn>
          auto DownscaleConverter<TOut, TIn>::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        template<typename TOut, typename TIn>
          void DownscaleConverter<TOut, TIn>::read(mods::utils::RWBuffer<u8>* buf, size_t len)
            {
               if((len % sizeof(TOut)) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in DownscaleWavConverter" << std::endl;
                 }
               
               int nbElems = len / sizeof(TOut);
               int toReadLen = nbElems * sizeof(TIn);
               
               ensureTempBufferSize(toReadLen);
               
               _src->read(&_temp, toReadLen);
               
               auto inView = _temp.slice<TIn>(0, nbElems);
               auto outView = buf->slice<TOut>(0, nbElems);
               
               for(int i = 0; i<nbElems; ++i)
               {
                  TIn value = inView[i];
                  outView[i] = static_cast<u32>(value) >> shiftRight();
               }
            }
        
        template<typename TOut, typename TIn>
          auto DownscaleConverter<TOut, TIn>::allocateNewTempBuffer(size_t len) -> mods::utils::RWBuffer<u8>
            {
               _tempVec.resize(len);
               u8* ptr = _tempVec.data();
               auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
               auto buffer = std::make_unique<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
               return mods::utils::RWBuffer<u8>(std::move(buffer));
            }
        
        template<typename TOut, typename TIn>
          void DownscaleConverter<TOut, TIn>::ensureTempBufferSize(size_t len)
            {
               if(_temp.size() < len)
                 {
                    _temp = allocateNewTempBuffer(len);
                 }
            }
        
        template class DownscaleConverter<s16, s32>;
     } // namespace converters
} // namespace mods
