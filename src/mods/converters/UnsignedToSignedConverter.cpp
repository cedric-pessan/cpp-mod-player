
#include "mods/converters/Converter.hpp"
#include "mods/converters/UnsignedToSignedConverter.hpp"
#include "mods/utils/RWBuffer.hpp"
#include "mods/utils/types.hpp"

#include <type_traits>

namespace mods
{
   namespace converters
     {
        template<typename TOut, typename TIn>
          UnsignedToSignedConverter<TOut, TIn>::UnsignedToSignedConverter(typename Converter<TIn>::ptr src)
            : _src(std::move(src))
              {
                 static_assert(sizeof(TOut) == sizeof(TIn), "Input and Output types of unsigned to signed converter should have the same size");
                 static_assert(std::is_signed<TOut>::value, "Output should be signed");
                 static_assert(std::is_unsigned<TIn>::value, "Input should be unsigned");
              }
        
        template<typename TOut, typename TIn>
          auto UnsignedToSignedConverter<TOut, TIn>::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        template<typename TOut, typename TIn>
          void UnsignedToSignedConverter<TOut, TIn>::read(mods::utils::RWBuffer<TOut>* buf)
            {
               auto inBuf = buf->template slice<TIn>(0, buf->size());
               
               _src->read(&inBuf);
               
               for(TOut& elem : *buf)
                 {
                    const u32 value = static_cast<u32>(static_cast<TIn>(elem));
                    const s32 svalue = static_cast<s32>(value);
                    elem = static_cast<TOut>(svalue - getOffset());
                 }
            }
        
        template class UnsignedToSignedConverter<s8, u8>;
     } // namespace converters
} // namespace mods
