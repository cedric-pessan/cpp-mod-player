##################
# module reader
cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 14)

project(modreader)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()

include(TestBigEndian)

TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

Find_Package(SDL REQUIRED)

include_directories(${SDL_INCLUDE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/include)

configure_file(include/mods/utils/nativeEndian.hpp.in include/mods/utils/nativeEndian.hpp)

add_executable(filter_generator src/mods/utils/Band.cpp
                                src/mods/utils/BufferBackend.cpp
                                src/mods/utils/FilterGeneratorTool.cpp
                                src/mods/utils/FirFilterDesigner.cpp)

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
                         src/mods/wav/ALawConverter.cpp
                         src/mods/wav/ChannelCopyWavConverter.cpp
                         src/mods/wav/DownscaleWavConverter.cpp
                         src/mods/wav/FillLSBConverter.cpp
                         src/mods/wav/Format.cpp
                         src/mods/wav/FromDoubleConverter.cpp
                         src/mods/wav/GSMDecoderConverter.cpp
                         src/mods/wav/MuLawConverter.cpp
                         src/mods/wav/MultiplexerWavConverter.cpp
                         src/mods/wav/ReaderWavConverter.cpp
                         src/mods/wav/ResampleConverter.cpp
                         src/mods/wav/ResamplePositiveIntegerFactor.cpp
                         src/mods/wav/StatCollector.cpp
                         src/mods/wav/ToDoubleConverter.cpp
                         src/mods/wav/UnpackToTypeConverter.cpp
                         src/mods/wav/UnsignedToSignedWavConverter.cpp
                         src/mods/wav/UpscaleWavConverter.cpp
                         src/mods/wav/WavConverter.cpp
                         src/mods/wav/WavReader.cpp
                         src/mods/wav/WavTypes.cpp
                         src/mods/utils/BitReader.cpp
                         src/mods/utils/BufferBackend.cpp
                         src/mods/utils/FileUtils.cpp
                         ${SRCS_UNIX}
                         ${CMAKE_CURRENT_BINARY_DIR}/include/mods/utils/Filters.hpp
                         ${CMAKE_CURRENT_BINARY_DIR}/src/mods/utils/Filters.cpp)

target_link_libraries(modreader ${SDL_LIBRARY})

set_property(TARGET filter_generator PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
set_property(TARGET modreader PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
