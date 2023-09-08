#ifndef MODS_CONVERTERS_OPENCLRESAMPLECONVERTER_HPP
#define MODS_CONVERTERS_OPENCLRESAMPLECONVERTER_HPP

#include "config.hpp"
#include "mods/converters/impl/OpenCLResampleConverterImpl.hpp"
#include "mods/utils/AmigaRLESample.hpp"
#include "mods/utils/OpenCLManager.hpp"

#ifdef WITH_OPENCL
namespace mods
{
   namespace converters
     {
        template<typename PARAMETERS, typename T>
          class OpenCLResampleConverter : public impl::OpenCLResampleConverterBase<PARAMETERS, T>
          {
           public:
             OpenCLResampleConverter(typename Converter<T>::ptr src, PARAMETERS resampleParameters);
             
             OpenCLResampleConverter() = delete;
             OpenCLResampleConverter(const OpenCLResampleConverter&) = delete;
             OpenCLResampleConverter(OpenCLResampleConverter&&) = delete;
             auto operator=(const OpenCLResampleConverter&) -> OpenCLResampleConverter& = delete;
             auto operator=(OpenCLResampleConverter&&) -> OpenCLResampleConverter& = delete;
             ~OpenCLResampleConverter() override = default;
             
             void read(mods::utils::RWBuffer<double>* buf) override;
             
           private:
             void fillBuffers(const cl::Context& context, const cl::CommandQueue& queue, cl::Buffer* sampleBuffer, cl::Buffer* zerosBuffer, cl::Buffer* repeatBuffer) override;
          };
        
        class AmigaResampleParameters;
        
        template<>
          class OpenCLResampleConverter<AmigaResampleParameters, mods::utils::AmigaRLESample> : public impl::OpenCLResampleConverterBase<AmigaResampleParameters, mods::utils::AmigaRLESample>
          {
           private:
             using RLESample = mods::utils::AmigaRLESample;
             
           public:
             OpenCLResampleConverter(Converter<RLESample>::ptr src, AmigaResampleParameters resampleParameters);
             
             OpenCLResampleConverter() = delete;
             OpenCLResampleConverter(const OpenCLResampleConverter&) = delete;
             OpenCLResampleConverter(OpenCLResampleConverter&&) = delete;
             auto operator=(const OpenCLResampleConverter&) -> OpenCLResampleConverter& = delete;
             auto operator=(OpenCLResampleConverter&&) -> OpenCLResampleConverter& = delete;
             ~OpenCLResampleConverter() override = default;
             
             void read(mods::utils::RWBuffer<double>* buf) override;
             
           private:
             auto buildFilteredTapsBuffer() -> cl::Buffer;
             void fillBuffers(const cl::Context& context, const cl::CommandQueue& queue, cl::Buffer* sampleBuffer, cl::Buffer* zerosBuffer, cl::Buffer* repeatBuffer) override;
             
             bool _filtered = false;
             std::vector<double> _filteredTaps;
             cl::Buffer _ledFilterTapsBuffer;
          };
     } // namespace converters
} // namespace mods
#endif // WITH_OPENCL

#endif // MODS_CONVERTERS_OPENCLRESAMPLECONVERTER_HPP
