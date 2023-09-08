#ifndef MODS_CONVERTERS_RESAMPLECONVERTER_HPP
#define MODS_CONVERTERS_RESAMPLECONVERTER_HPP

#include "mods/converters/Converter.hpp"
#include "mods/converters/impl/ResampleConverterImpl.hpp"

namespace mods
{
   namespace converters
     {
        template<typename PARAMETERS, typename T>
          class ResampleConverter : public mods::converters::Converter<double>
          {
           public:
             ResampleConverter(typename Converter<T>::ptr src, PARAMETERS parameters);
             
             ResampleConverter() = delete;
             ResampleConverter(const ResampleConverter&) = delete;
             ResampleConverter(ResampleConverter&&) = delete;
             auto operator=(const ResampleConverter&) -> ResampleConverter& = delete;
             auto operator=(ResampleConverter&&) -> ResampleConverter& = delete;
             ~ResampleConverter() override = default;
             
             auto isFinished() const -> bool override;
             
           protected:
             void removeFromHistory();
             void addToHistory();
             
             auto getHistory() -> impl::History&;
             auto getResampleParameters() const -> const PARAMETERS&;
             
           private:
             auto getNextSample() -> double;
             auto nextSampleExists() const -> bool;
             auto currentSampleRemainingDuration() const -> size_t;
             auto initBuffer() -> mods::utils::RWBuffer<T>;
             auto shouldDedup() const -> bool;
             
             PARAMETERS _resampleParameters;
             impl::History _history;
             
             typename Converter<T>::ptr _src;
             int _zerosToNextInterpolatedSample = 0;
             
             std::vector<u8> _inputVec;
             mods::utils::RWBuffer<T> _inputBuffer;
             size_t _currentSample;
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_RESAMPLECONVERTER_HPP
