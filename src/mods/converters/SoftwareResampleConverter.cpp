
#include "mods/StandardFrequency.hpp"
#include "mods/converters/Converter.hpp"
#include "mods/converters/ResampleConverter.hpp"
#include "mods/converters/ResampleParameters.hpp"
#include "mods/converters/SoftwareResampleConverter.hpp"
#include "mods/converters/impl/ResampleConverterImpl.hpp"
#include "mods/utils/AmigaRLESample.hpp"
#include "mods/utils/RWBuffer.hpp"

#include <cstddef>

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
               for(auto& elem : *buf)
                 {
                    if(this->isFinished())
                      {
                         elem = 0.0;
                      }
                    else
                      {
                         elem = getNextDecimatedSample();
                      }
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
          auto SoftwareResampleConverter<PARAMETERS, T>::getTap(size_t tapIndex, const impl::SampleWithZeros& /* sample */) const -> double
          {
             auto& resampleParameters = this->getResampleParameters();
             
             return resampleParameters.getTap(tapIndex);
          }
        
        template<>
          auto SoftwareResampleConverter<AmigaResampleParameters, mods::utils::AmigaRLESample>::getTap(size_t tapIndex, const impl::SampleWithZeros& sample) const -> double
          {
             if(sample.isFiltered())
               {
                  return AmigaResampleParameters::getFilteredTap(tapIndex);
               }
             return AmigaResampleParameters::getTap(tapIndex);
          }
        
        template<typename PARAMETERS, typename T>
          auto SoftwareResampleConverter<PARAMETERS, T>::calculateInterpolatedSample() -> double
          {
             auto& resampleParameters = this->getResampleParameters();
             auto& history = this->getHistory();
             
             double sample = 0.0;
             int idxSampleWithZeros = 0;
             
             auto interpolationFactor = resampleParameters.getInterpolationFactor();
             for(size_t i = 0; i < resampleParameters.getNumTaps(); ++i) 
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
             auto& history = this->getHistory();
             
             double sample = 0.0;
             int idxSampleWithZeros = 0;
             
             auto interpolationFactor = static_cast<double>(AmigaResampleParameters::getInterpolationFactor());
             for(int i = 0; i < AmigaResampleParameters::getNumTaps(); ++i) 
               {
                  const auto& sampleWithZeros = history.getSample(idxSampleWithZeros++);
                  for(int j = 0; j < sampleWithZeros.getRepeatCount() && i < AmigaResampleParameters::getNumTaps(); ++j)
                    {
                       i += sampleWithZeros.getNumberOfZeros();
                       if(i < AmigaResampleParameters::getNumTaps()) 
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
