
#include "mods/wav/DownscaleWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        template<typename TOut, typename TIn>
          DownscaleWavConverter<TOut, TIn>::DownscaleWavConverter(WavConverter::ptr src)
            : _src(std::move(src)),
          _temp(allocateNewTempBuffer(0))
              {
                 if(sizeof(TOut) > sizeof(TIn))
                   {
                      std::cout << "TOut should be smaller than TIn in DownscaleWavConverter" << std::endl;
                   }
              }
        
        template<typename TOut, typename TIn>
          bool DownscaleWavConverter<TOut, TIn>::isFinished() const
          {
             return _src->isFinished();
          }
        
        template<typename TOut, typename TIn>
          void DownscaleWavConverter<TOut, TIn>::read(mods::utils::RWBuffer<u8>* buf, int len)
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
          mods::utils::RWBuffer<u8> DownscaleWavConverter<TOut, TIn>::allocateNewTempBuffer(size_t len)
            {
               _tempVec.resize(len);
               u8* ptr = _tempVec.data();
               auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
               auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, len, std::move(deleter));
               return mods::utils::RWBuffer<u8>(buffer);
            }
        
        template<typename TOut, typename TIn>
          void DownscaleWavConverter<TOut, TIn>::ensureTempBufferSize(size_t len)
            {
               if(_temp.size() < len)
                 {
                    _temp = allocateNewTempBuffer(len);
                 }
            }
        
        template class DownscaleWavConverter<s16, s32>;
     } // namespace wav
} // namespace mods
