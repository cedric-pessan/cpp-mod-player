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

set(SRCS_UNIX src/mods/utils/impl/unix/FileUtils.cpp
              src/mods/utils/impl/unix/FileUtils.c)

add_executable(modreader src/main.cpp
                         src/mods/ModuleReader.cpp
                         src/mods/SoundPlayer.cpp
                         src/mods/wav/WavConverter.cpp
                         src/mods/wav/WavReader.cpp
                         src/mods/wav/MultiplexerWavConverter.cpp
                         src/mods/wav/DummyWavConverter.cpp
                         src/mods/utils/Buffer.cpp
                         src/mods/utils/FileUtils.cpp
                         ${SRCS_UNIX})

target_link_libraries(modreader ${SDL_LIBRARY})

set_property(TARGET modreader PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
