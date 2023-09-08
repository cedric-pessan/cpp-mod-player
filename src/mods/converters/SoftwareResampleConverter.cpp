
#include "mods/converters/ResampleParameters.hpp"
#include "mods/converters/SoftwareResampleConverter.hpp"
#include "mods/utils/AmigaRLESample.hpp"

namespace mods
{
   namespace converters
     {
        template<typename PARAMETERS, typename T>
          SoftwareResampleConverter<PARAMETERS, T>::SoftwareResampleConverter(typename Converter<T>::ptr src, PARAMETERS resampleParameters)
            : ResampleConverter<PARAMETERS, T>(std::move(src), std::move(resampleParameters))
              {
              }
        
        template<typename PARAMETERS, typename T>
          void SoftwareResampleConverter<PARAMETERS, T>::read(mods::utils::RWBuffer<double>* buf)
            {
               for(size_t i=0; i<buf->size(); ++i)
                 {
                    (*buf)[i] = getNextDecimatedSample();
                 }
            }
        
        template<typename PARAMETERS, typename T>
          auto SoftwareResampleConverter<PARAMETERS, T>::getNextDecimatedSample() -> double
            {
               updateHistory();
               return calculateInterpolatedSample();
            }
        
        template<typename PARAMETERS, typename T>
          void SoftwareResampleConverter<PARAMETERS, T>::updateHistory()
            {
               this->removeFromHistory();
               this->addToHistory();
            }
        
        template<typename PARAMETERS, typename T>
          auto SoftwareResampleConverter<PARAMETERS, T>::getTap(size_t i, const impl::SampleWithZeros& sample) const -> double
          {
             auto& resampleParameters = this->getResampleParameters();
             
             return resampleParameters.getTap(i);
          }
        
        template<>
          auto SoftwareResampleConverter<AmigaResampleParameters, mods::utils::AmigaRLESample>::getTap(size_t i, const impl::SampleWithZeros& sample) const -> double
          {
             auto& resampleParameters = getResampleParameters();
             
             if(sample.isFiltered())
               {
                  return resampleParameters.getFilteredTap(i);
               }
             else
               {
                  return resampleParameters.getTap(i);
               }
          }
        
        template<typename PARAMETERS, typename T>
          auto SoftwareResampleConverter<PARAMETERS, T>::calculateInterpolatedSample() -> double
          {
             auto& resampleParameters = this->getResampleParameters();
             auto& history = this->getHistory();
             
             double sample = 0.0;
             int idxSampleWithZeros = 0;
             
             auto interpolationFactor = resampleParameters.getInterpolationFactor();
             for(int i = 0; i < resampleParameters.getNumTaps(); ++i) 
               {
                  const auto& sampleWithZeros = history.getSample(idxSampleWithZeros++);
                  i += sampleWithZeros.getNumberOfZeros();
                  if(i < resampleParameters.getNumTaps()) 
                    {
                       sample += sampleWithZeros.getSample() * interpolationFactor * getTap(i, sampleWithZeros);
                    }
               }
             return sample;
          }
        
        template<>
          auto SoftwareResampleConverter<AmigaResampleParameters, mods::utils::AmigaRLESample>::calculateInterpolatedSample() -> double
          {
             auto& resampleParameters = this->getResampleParameters();
             auto& history = this->getHistory();
             
             double sample = 0.0;
             int idxSampleWithZeros = 0;
             
             auto interpolationFactor = resampleParameters.getInterpolationFactor();
             for(int i = 0; i < resampleParameters.getNumTaps(); ++i) 
               {
                  const auto& sampleWithZeros = history.getSample(idxSampleWithZeros++);
                  for(int j = 0; j < sampleWithZeros.getRepeatCount() && i < resampleParameters.getNumTaps(); ++j)
                    {
                       i += sampleWithZeros.getNumberOfZeros();
                       if(i < resampleParameters.getNumTaps()) 
                         {
                            sample += sampleWithZeros.getSample() * interpolationFactor * getTap(i, sampleWithZeros);
                         }
                    }
               }
             return sample;
          }
        
        template class SoftwareResampleConverter<StaticResampleParameters<StandardFrequency::_22000, StandardFrequency::_44100>, double>;
        template class SoftwareResampleConverter<StaticResampleParameters<StandardFrequency::_8000,  StandardFrequency::_44100>, double>;
        template class SoftwareResampleConverter<StaticResampleParameters<StandardFrequency::_48000, StandardFrequency::_44100>, double>;
	template class SoftwareResampleConverter<StaticResampleParameters<StandardFrequency::_10000, StandardFrequency::_44100>, double>;
        template class SoftwareResampleConverter<DynamicResampleParameters, double>;
        template class SoftwareResampleConverter<AmigaResampleParameters, mods::utils::AmigaRLESample>;
     } // namespace converters
} // namespace mods
