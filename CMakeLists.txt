cmake_minimum_required(VERSION 3.25)
project(GBpp)
set(CMAKE_CXX_STANDARD 23)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(GBpp src/main.cpp src/gameboy.cpp src/opcode.cpp
        src/interupts.cpp src/ppu.cpp)
target_link_libraries(GBpp ${SDL2_LIBRARIES})