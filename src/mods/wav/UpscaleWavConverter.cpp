
#include "mods/wav/UpscaleWavConverter.hpp"

#include <iostream>

namespace mods
{
   namespace wav
     {
        template<typename TOut, typename TIn>
          UpscaleWavConverter<TOut, TIn>::UpscaleWavConverter(WavConverter::ptr src)
            : _src(std::move(src))
              {
              }
        
        template<typename TOut, typename TIn>
          bool UpscaleWavConverter<TOut, TIn>::isFinished() const
          {
             return _src->isFinished();
          }
        
        template<typename TOut, typename TIn>
          void UpscaleWavConverter<TOut, TIn>::read(mods::utils::RWBuffer<u8>& buf, int len)
            {
               if((len % sizeof(TOut)) != 0)
                 std::cout << "TODO: wrong buffer length in UpscaleWavConverter" << std::endl;
               if(sizeof(TOut) > 2*sizeof(TIn))
                 std::cout << "TODO: out format too large" << std::endl;
               
               int nbElems = len / sizeof(TOut);
               int toReadLen = nbElems * sizeof(TIn);
               
               _src->read(buf, toReadLen);
               
               auto inView = buf.slice<TIn>(0, nbElems);
               auto outView = buf.slice<TOut>(0, nbElems);
               
               for(int i = nbElems-1; i>=0; --i)
                 {
                    TOut value = inView[i];
                    value = (value << shiftLeftValue()) | ((value >> shiftRightValue()) & maskValue());
                    outView[i] = value;
                 }
            }
        
        template class UpscaleWavConverter<s16, s8>;
     } // namespace wav
} // namespace mods
