##################
# module reader
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)

project(modreader)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()

include(TestBigEndian)

TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

Find_Package(SDL2 REQUIRED)
Find_Package(OpenCL)

if(OpenCL_FOUND)
set(WITH_OPENCL 1)
endif()

include_directories(${SDL2_INCLUDE_DIR}
                    ${OpenCL_INCLUDE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/include)
                    
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/contrib/include)

configure_file(include/mods/utils/nativeEndian.hpp.in include/mods/utils/nativeEndian.hpp)
configure_file(include/config.hpp.in include/config.hpp)

add_executable(filter_generator src/mods/tools/FilterGeneratorTool.cpp
                                src/mods/utils/bessel.cpp
                                src/mods/utils/FirFilterDesigner.cpp
                                src/mods/utils/RBufferBackend.cpp)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src/mods/utils")

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/include/mods/utils/Filters.hpp"
                          "${CMAKE_CURRENT_BINARY_DIR}/src/mods/utils/Filters.cpp"
                   COMMAND filter_generator "${CMAKE_CURRENT_BINARY_DIR}/include/mods/utils/Filters.hpp" 
                                            "${CMAKE_CURRENT_BINARY_DIR}/src/mods/utils/Filters.cpp"
                   DEPENDS filter_generator
                   COMMENT "Generate filters")

set(SRCS_UNIX src/mods/utils/impl/unix/FileUtils.cpp
              src/mods/utils/impl/unix/FileUtils.c)

add_executable(modreader src/main.cpp
                         src/mods/ModuleReader.cpp
                         src/mods/SoundPlayer.cpp
                         src/mods/converters/CastConverter.cpp
                         src/mods/converters/ChannelCopyConverter.cpp
                         src/mods/converters/DivideConverter.cpp
                         src/mods/converters/DownscaleConverter.cpp
                         src/mods/converters/FillLSBConverter.cpp
                         src/mods/converters/FromDoubleConverter.cpp
                         src/mods/converters/MultiplexerConverter.cpp
                         src/mods/converters/OpenCLResampleConverter.cpp
                         src/mods/converters/ResampleConverter.cpp
                         src/mods/converters/ResampleParameters.cpp
                         src/mods/converters/ResamplePositiveIntegerFactor.cpp
                         src/mods/converters/SoftwareResampleConverter.cpp
                         src/mods/converters/ToDoubleConverter.cpp
                         src/mods/converters/UnsignedToSignedConverter.cpp
                         src/mods/converters/UpscaleConverter.cpp
                         src/mods/mod/Arpeggio.cpp
                         src/mods/mod/ChannelState.cpp
                         src/mods/mod/Instrument.cpp
                         src/mods/mod/ModChannelConverter.cpp
                         src/mods/mod/ModReader.cpp
                         src/mods/mod/NoEffect.cpp
                         src/mods/mod/Note.cpp
                         src/mods/mod/PatternListReader.cpp
                         src/mods/mod/PatternReader.cpp
                         src/mods/mod/SlideDown.cpp
                         src/mods/mod/SlideUp.cpp
                         src/mods/mod/Vibrato.cpp
                         src/mods/mod/VibratoAndVolumeSlide.cpp
                         src/mods/mod/VolumeSlide.cpp
                         src/mods/wav/ADPCMDecoderConverter.cpp
                         src/mods/wav/ALawConverter.cpp
                         src/mods/wav/DemuxConverter.cpp
			 src/mods/wav/DVIADPCMDecoderConverter.cpp
                         src/mods/wav/Format.cpp
                         src/mods/wav/GSMDecoderConverter.cpp
                         src/mods/wav/MuLawConverter.cpp
                         src/mods/wav/MultiChannelMixer.cpp
                         src/mods/wav/OKIADPCMDecoderConverter.cpp
                         src/mods/wav/ReaderWavConverter.cpp
                         src/mods/wav/StatCollector.cpp
                         src/mods/wav/TruspeechDecoderConverter.cpp
                         src/mods/wav/UnpackToTypeConverter.cpp
                         src/mods/wav/WavConverterFactory.cpp
                         src/mods/wav/WavReader.cpp
                         src/mods/wav/WavTypes.cpp
                         src/mods/utils/AmigaRLESample.cpp
                         src/mods/utils/bessel.cpp
                         src/mods/utils/BitReader.cpp
                         src/mods/utils/OpenCLManager.cpp
                         src/mods/utils/FileUtils.cpp
			 src/mods/utils/FirFilterDesigner.cpp
                         src/mods/utils/RBufferBackend.cpp
                         src/mods/utils/RWBufferBackend.cpp
                         ${SRCS_UNIX}
                         ${CMAKE_CURRENT_BINARY_DIR}/include/mods/utils/Filters.hpp
                         ${CMAKE_CURRENT_BINARY_DIR}/src/mods/utils/Filters.cpp)

target_link_libraries(modreader ${SDL2_LIBRARIES} ${OpenCL_LIBRARY})

set_property(TARGET filter_generator PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
set_property(TARGET modreader PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
