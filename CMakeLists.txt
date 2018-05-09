##################
# module reader
cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 14)

project(modreader)

add_executable(modreader src/main.cpp
                         src/ModReader.cpp)
