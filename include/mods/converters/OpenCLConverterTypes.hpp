#ifndef MODS_CONVERTERS_OPENCLCONVERTERTYPES_HPP
#define MODS_CONVERTERS_OPENCLCONVERTERTYPES_HPP

#include "config.hpp"

#include "mods/converters/OpenCLResampleConverter.hpp"
#include "mods/converters/ResampleConverter.hpp"

namespace mods
{
   namespace converters
     {
        template<typename PARAMETERS>
          class OpenCLConverterTypes
          {
           public:
             OpenCLConverterTypes() = delete;
             OpenCLConverterTypes(const OpenCLConverterTypes&) = delete;
             OpenCLConverterTypes(OpenCLConverterTypes&&) = delete;
             auto operator=(const OpenCLConverterTypes&) -> OpenCLConverterTypes& = delete;
             auto operator=(OpenCLConverterTypes&&) -> OpenCLConverterTypes& = delete;
             ~OpenCLConverterTypes() = delete;
             
#ifdef WITH_OPENCL
             using ResampleConverterImpl = OpenCLResampleConverter<PARAMETERS>;
#else // WITH_OPENCL
             using ResampleConverterImpl = SoftwareResampleConverter<PARAMETERS>;
#endif // WITH_OPENCL
             
          };
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_OPENCLCONVERTERTYPES_HPP
