##################
# module reader
cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 14)

project(modreader)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

Find_Package(SDL REQUIRED)

include_directories(${SDL_INCLUDE_DIR})

set(SRCS_UNIX src/impl/unix/FileUtils.cpp)

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
