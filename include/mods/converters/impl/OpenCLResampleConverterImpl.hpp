#ifndef MODS_CONVERTERS_IMPL_OPENCLRESAMPLECONVERTERIMPL_HPP
#define MODS_CONVERTERS_IMPL_OPENCLRESAMPLECONVERTERIMPL_HPP

#include "config.hpp"
#include "mods/converters/ResampleConverter.hpp"
#include "mods/converters/ResampleParameters.hpp"
#include "mods/utils/OpenCLManager.hpp"

#ifdef WITH_OPENCL
namespace mods
{
   namespace converters
     {
        namespace impl
          {
             template<typename PARAMETERS, typename T>
               class OpenCLResampleConverterBase : public ResampleConverter<PARAMETERS, T>
               {
                public:
                  OpenCLResampleConverterBase(typename Converter<T>::ptr src, PARAMETERS resampleParameters, bool rleInput);
                  
                  OpenCLResampleConverterBase() = delete;
                  OpenCLResampleConverterBase(const OpenCLResampleConverterBase&) = delete;
                  OpenCLResampleConverterBase(OpenCLResampleConverterBase&&) = delete;
                  auto operator=(const OpenCLResampleConverterBase&) -> OpenCLResampleConverterBase& = delete;
                  auto operator=(OpenCLResampleConverterBase&&) -> OpenCLResampleConverterBase& = delete;
                  ~OpenCLResampleConverterBase() override = default;
                  
                  void resample(mods::utils::RWBuffer<double>* buf, const cl::Buffer& tapsBuffer);
                  
                protected:
                  auto getTapsBuffer() const -> const cl::Buffer&;
                  
                private:
                  auto compileFirFilterProgram(bool rleInput) -> cl::Program;
                  auto buildTapsBuffer() -> cl::Buffer;
                  
                  virtual void fillSampleBuffer(const cl::Context& context, const cl::CommandQueue& queue, cl::Buffer* sampleBuffer) = 0;
                  virtual void fillZerosBuffer(const cl::Context& context, const cl::CommandQueue& queue, cl::Buffer* zerosBuffer) = 0;
                  virtual void fillRepeatBuffer(const cl::Context& context, const cl::CommandQueue& queue, cl::Buffer* repeatBuffer) = 0;
                  
                  cl::Context _context;
                  cl::CommandQueue _queue;
                  cl::Program _firFilterProgram;
                  cl::KernelFunctor<cl::Buffer, cl::Buffer, cl::Buffer, cl::Buffer, const cl::Buffer&> _firFilterKernel;
                  std::vector<double> _taps;
                  cl::Buffer _tapsBuffer;
                  
                  cl::Buffer _sampleBuffer;
                  cl::Buffer _zerosBuffer;
                  cl::Buffer _repeatBuffer;
                  cl::Buffer _outputBuffer;
               };
          } // namespace impl
     } // namespace converters
} // namespace mods
#endif // WITH_OPENCL

#endif // MODS_CONVERTERS_IMPL_OPENCLRESAMPLECONVERTERIMPL_HPP
