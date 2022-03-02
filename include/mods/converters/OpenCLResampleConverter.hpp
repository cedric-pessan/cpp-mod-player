#ifndef MODS_CONVERTERS_OPENCLRESAMPLECONVERTER_HPP
#define MODS_CONVERTERS_OPENCLRESAMPLECONVERTER_HPP

#include "config.hpp"
#include "mods/converters/ResampleConverter.hpp"
#include "mods/utils/OpenCLManager.hpp"

#ifdef WITH_OPENCL
namespace mods
{
   namespace converters
     {
        template<typename PARAMETERS>
          class OpenCLResampleConverter : public ResampleConverter<PARAMETERS>
          {
           public:
             OpenCLResampleConverter(Converter::ptr src, PARAMETERS resampleParameters);
             
             OpenCLResampleConverter() = delete;
             OpenCLResampleConverter(const OpenCLResampleConverter&) = delete;
             OpenCLResampleConverter(OpenCLResampleConverter&&) = delete;
             auto operator=(const OpenCLResampleConverter&) -> OpenCLResampleConverter& = delete;
             auto operator=(OpenCLResampleConverter&&) -> OpenCLResampleConverter& = delete;
             ~OpenCLResampleConverter() override = default;
             
             void read(mods::utils::RWBuffer<u8>* buf, size_t len) override;
             
           private:
             auto compileFirFilterProgram() -> cl::Program;
             auto buildTapsBuffer() -> cl::Buffer;
             
             cl::Context _context;
             cl::CommandQueue _queue;
             cl::Program _firFilterProgram;
             cl::KernelFunctor<cl::Buffer, cl::Buffer, cl::Buffer, const cl::Buffer&> _firFilterKernel;
             std::vector<double> _taps;
             cl::Buffer _tapsBuffer;
          };
     } // namespace converters
} // namespace mods
#endif // WITH_OPENCL

#endif // MODS_CONVERTERS_OPENCLRESAMPLECONVERTER_HPP
