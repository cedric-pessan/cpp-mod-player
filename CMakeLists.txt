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
                    ${CMAKE_BINARY_DIR}/include
                    ${CMAKE_SOURCE_DIR}/src)

configure_file(src/nativeEndian.hpp.in include/nativeEndian.hpp)

set(SRCS_UNIX src/impl/unix/FileUtils.cpp
              src/impl/unix/modopen.c)

add_executable(modreader src/main.cpp
                         src/ModuleReader.cpp
                         src/SoundPlayer.cpp
                         src/WavConverter.cpp
                         src/WavReader.cpp
                         src/Buffer.cpp
                         src/FileUtils.cpp
                         ${SRCS_UNIX})

target_link_libraries(modreader ${SDL_LIBRARY})

set_property(TARGET modreader PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
