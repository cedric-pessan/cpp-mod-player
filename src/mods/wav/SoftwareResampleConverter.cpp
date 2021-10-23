
#include "mods/wav/ResampleParameters.hpp"
#include "mods/wav/SoftwareResampleConverter.hpp"

namespace mods
{
   namespace wav
     {
        template<typename PARAMETERS>
          SoftwareResampleConverter<PARAMETERS>::SoftwareResampleConverter(WavConverter::ptr src, PARAMETERS resampleParameters)
            : ResampleConverter<PARAMETERS>(std::move(src), std::move(resampleParameters))
            {
            }
        
        template<typename PARAMETERS>
          void SoftwareResampleConverter<PARAMETERS>::read(mods::utils::RWBuffer<u8>* buf, size_t len)
            {
               if((len % sizeof(double)) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in ResampleConverter" << std::endl;
                 }
               
               auto nbElems = len / sizeof(double);
               
               auto outView = buf->slice<double>(0, nbElems);
               
               for(size_t i=0; i<nbElems; ++i)
                 {
                    outView[i] = getNextDecimatedSample();
                 }
            }
        
        template<typename PARAMETERS>
          auto SoftwareResampleConverter<PARAMETERS>::getNextDecimatedSample() -> double
            {
               updateHistory();
               return calculateInterpolatedSample();
            }
        
        template<typename PARAMETERS>
          void SoftwareResampleConverter<PARAMETERS>::updateHistory()
            {
               ResampleConverter<PARAMETERS>::removeFromHistory();
               ResampleConverter<PARAMETERS>::addToHistory();
            }
        
        template<typename PARAMETERS>
          auto SoftwareResampleConverter<PARAMETERS>::calculateInterpolatedSample() -> double
          {
             auto& resampleParameters = ResampleConverter<PARAMETERS>::_resampleParameters;
             auto& history = ResampleConverter<PARAMETERS>::_history;
             
             double sample = 0.0;
             int idxSampleWithZeros = 0;
             
             auto interpolationFactor = resampleParameters.getInterpolationFactor();
             for(int i = 0; i < resampleParameters.getNumTaps(); ++i) 
               {
                  const auto& sampleWithZeros = history.getSample(idxSampleWithZeros++);
                  i += sampleWithZeros.numberOfZeros();
                  if(i < resampleParameters.getNumTaps()) 
                    {
                       sample += sampleWithZeros.sample() * interpolationFactor * resampleParameters.getTap(i);
                    }
               }
             return sample;
          }
        
        template class SoftwareResampleConverter<StaticResampleParameters<StandardFrequency::_22000, StandardFrequency::_44100>>;
        template class SoftwareResampleConverter<StaticResampleParameters<StandardFrequency::_8000,  StandardFrequency::_44100>>;
        template class SoftwareResampleConverter<StaticResampleParameters<StandardFrequency::_48000, StandardFrequency::_44100>>;
	template class SoftwareResampleConverter<StaticResampleParameters<StandardFrequency::_10000, StandardFrequency::_44100>>;
        template class SoftwareResampleConverter<DynamicResampleParameters>;
     } // namespace wav
} // namespace mods
