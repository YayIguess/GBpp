cmake_minimum_required(VERSION 3.25)
project(GameBoy++)
set(CMAKE_CXX_STANDARD 23)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(GameBoy++ src/main.cpp
        src/gameboy.cpp
        src/opcodeResolver.cpp
        src/interupts.cpp
        src/ppu.cpp
        src/timing.cpp
        src/extendedOpcodeResolver.cpp
        src/mbc.cpp
        src/addressSpace.cpp
        src/addressSpace.hpp
        src/testing.hpp
        src/joypad.cpp
)
target_link_libraries(GameBoy++ ${SDL2_LIBRARIES})