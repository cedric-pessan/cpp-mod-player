
#include "mods/converters/ResampleParameters.hpp"
#include "mods/converters/SoftwareResampleConverter.hpp"

namespace mods
{
   namespace converters
     {
        template<typename PARAMETERS>
          SoftwareResampleConverter<PARAMETERS>::SoftwareResampleConverter(Converter<double>::ptr src, PARAMETERS resampleParameters)
            : ResampleConverter<PARAMETERS>(std::move(src), std::move(resampleParameters))
            {
            }
        
        template<typename PARAMETERS>
          void SoftwareResampleConverter<PARAMETERS>::read(mods::utils::RWBuffer<double>* buf)
            {
               for(size_t i=0; i<buf->size(); ++i)
                 {
                    (*buf)[i] = getNextDecimatedSample();
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
             auto& resampleParameters = ResampleConverter<PARAMETERS>::getResampleParameters();
             auto& history = ResampleConverter<PARAMETERS>::getHistory();
             
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
     } // namespace converters
} // namespace mods
