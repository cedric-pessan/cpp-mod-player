#ifndef MODS_WAV_RESAMPLECONVERTER_HPP
#define MODS_WAV_RESAMPLECONVERTER_HPP

#include "mods/wav/WavConverter.hpp"
#include "mods/wav/impl/ResampleConverterImpl.hpp"

namespace mods
{
   namespace wav
     {
        template<typename PARAMETERS>
          class ResampleConverter : public WavConverter
          {
           public:
             ResampleConverter(WavConverter::ptr src, PARAMETERS parameters);
             
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
             auto initBuffer() -> mods::utils::RWBuffer<u8>;
             
             PARAMETERS _resampleParameters;
             impl::History _history;
             
             WavConverter::ptr _src;
             int _zerosToNextInterpolatedSample = 0;
             
             std::vector<u8> _inputVec;
             mods::utils::RWBuffer<u8> _inputBuffer;
             mods::utils::RWBuffer<double> _inputBufferAsDouble;
             size_t _currentSample;
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_RESAMPLECONVERTER_HPP
