#ifndef MODS_WAV_OPENCLCONVERTERTYPES_HPP
#define MODS_WAV_OPENCLCONVERTERTYPES_HPP

#include "config.hpp"

#include "mods/wav/OpenCLResampleConverter.hpp"
#include "mods/wav/ResampleConverter.hpp"

namespace mods
{
   namespace wav
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
             using ResampleConverterImpl = ResampleConverter<PARAMETERS>;
#endif // WITH_OPENCL
             
          };
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_OPENCLCONVERTERTYPES_HPP
