#ifndef MODS_WAV_OPENCLRESAMPLECONVERTER_HPP
#define MODS_WAV_OPENCLRESAMPLECONVERTER_HPP

#include "config.hpp"
#include "mods/wav/ResampleConverter.hpp"

#ifdef WITH_OPENCL
namespace mods
{
   namespace wav
     {
        template<typename PARAMETERS>
          class OpenCLResampleConverter : public ResampleConverter<PARAMETERS>
          {
           public:
             OpenCLResampleConverter(WavConverter::ptr src, PARAMETERS resampleParameters);
             
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
             cl::Buffer _tapsBuffer;
          };
     } // namespace wav
} // namespace mods
#endif // WITH_OPENCL

#endif // MODS_WAV_OPENCLRESAMPLECONVERTER_HPP
