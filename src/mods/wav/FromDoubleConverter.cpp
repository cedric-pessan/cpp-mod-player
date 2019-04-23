
#include "mods/wav/FromDoubleConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename T>
          FromDoubleConverter<T>::FromDoubleConverter(WavConverter::ptr src)
            : _src(std::move(src)),
          _temp(allocateTempBuffer(0))
            {
               if(sizeof(T) > sizeof(double))
                 {
                    std::cout << "TODO: very large data type to convert" << std::endl;
                 }
            }
        
        template<typename T>
          bool FromDoubleConverter<T>::isFinished() const
          {
             return _src->isFinished();
          }
        
        template<typename T>
          void FromDoubleConverter<T>::read(mods::utils::RWBuffer<u8>* buf, int len)
            {
               if((len % sizeof(T)) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in FromDoubleConverter" << std::endl;
                 }
               
               int nbElems = len / sizeof(T);
               int toReadLen = nbElems * sizeof(double);
               
               allocateTempBuffer(toReadLen);
               
               _src->read(&_temp, toReadLen);
               
               auto inView = _temp.slice<double>(0, nbElems);
               auto outView = buf->slice<T>(0, nbElems);
               
               for(int i = 0; i<nbElems; ++i)
                 {
                    double value = inView[i];
                    std::cout << "input value:" << value << std::endl;
                    if(value >= 1.0) value = 1.0;
                    if(value <= -1.0) value = -1.0;
                    if(value >= 0.0) value *= static_cast<double>(std::numeric_limits<T>::max());
                    else value *= -static_cast<double>(std::numeric_limits<T>::min());
                    T intValue = static_cast<T>(value);
                    outView[i] = intValue;
               }
            }
        
        template<typename T>
          mods::utils::RWBuffer<u8> FromDoubleConverter<T>::allocateTempBuffer(size_t len)
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
        
        template class FromDoubleConverter<s16>;
     } // namespace wav
} // namespace mods
