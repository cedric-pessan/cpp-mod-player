
#include "mods/wav/OpenCLResampleConverter.hpp"
#include "mods/wav/ResampleParameters.hpp"

namespace mods
{
   namespace wav
     {
        template<typename PARAMETERS>
          OpenCLResampleConverter<PARAMETERS>::OpenCLResampleConverter(WavConverter::ptr src, PARAMETERS parameters)
            {
            }
        
        template<typename PARAMETERS>
          auto OpenCLResampleConverter<PARAMETERS>::isFinished() const -> bool
          {
             std::cout << "TODO: OpenCLResampleConverter<PARAMETERS>::isFinished() const" << std::endl;
             return false;
          }
        
        template<typename PARAMETERS>
          void OpenCLResampleConverter<PARAMETERS>::read(mods::utils::RWBuffer<u8>* buf, size_t len)
            {
               std::cout << "TODO: OpenCLResampleConverter<PARAMETERS>::read(....)" << std::endl;
            }
        
        template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_22000, StandardFrequency::_44100>>;
        template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_8000,  StandardFrequency::_44100>>;
        template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_48000, StandardFrequency::_44100>>;
	template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_10000, StandardFrequency::_44100>>;
        template class OpenCLResampleConverter<DynamicResampleParameters>;
     } // namespace wav
} // namespace mods
