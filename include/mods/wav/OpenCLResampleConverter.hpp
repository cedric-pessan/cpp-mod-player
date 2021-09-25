#ifndef MODS_WAV_OPENCLRESAMPLECONVERTER_HPP
#define MODS_WAV_OPENCLRESAMPLECONVERTER_HPP

#include "config.hpp"
#include "mods/wav/WavConverter.hpp"
#include "mods/wav/impl/ResampleConverterImpl.hpp"

#ifdef WITH_OPENCL
namespace mods
{
   namespace wav
     {
        template<typename PARAMETERS>
          class OpenCLResampleConverter : public WavConverter
          {
           public:
             OpenCLResampleConverter(WavConverter::ptr src, PARAMETERS resampleParameters);
             
             OpenCLResampleConverter() = delete;
             OpenCLResampleConverter(const OpenCLResampleConverter&) = delete;
             OpenCLResampleConverter(OpenCLResampleConverter&&) = delete;
             auto operator=(const OpenCLResampleConverter&) -> OpenCLResampleConverter& = delete;
             auto operator=(OpenCLResampleConverter&&) -> OpenCLResampleConverter& = delete;
             ~OpenCLResampleConverter() override = default;
             
             auto isFinished() const -> bool override;
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             auto initBuffer() -> mods::utils::RWBuffer<u8>;
             
             auto compileFirFilterProgram() -> cl::Program;
             auto buildTapsBuffer() -> cl::Buffer;
             
             void addToHistory();
             void removeFromHistory();
             auto nextSampleExists() const -> bool;
             auto getNextSample() -> double;
             
             WavConverter::ptr _src;
             PARAMETERS _resampleParameters;
             int _zerosToNextInterpolatedSample = 0;
             
             std::vector<u8> _inputVec;
             mods::utils::RWBuffer<u8> _inputBuffer;
             mods::utils::RWBuffer<double> _inputBufferAsDouble;
             
             size_t _currentSample;
             
             impl::History _history;
             
             cl::Context _context;
             cl::CommandQueue _queue;
             cl::Program _firFilterProgram;
             cl::KernelFunctor<cl::Buffer, cl::Buffer, cl::Buffer, const cl::Buffer&> _firFilterKernel;
             cl::Buffer _tapsBuffer;
          };
     } // namespace wav
} // namespace mods
#endif // WITH_OPENCL

#endif // MODS_WAV_OPENCLRESAMPLECONVERTER_HPP
