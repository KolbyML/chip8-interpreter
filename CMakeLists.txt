cmake_minimum_required(VERSION 3.21)
project(chip8_emulator)

set(CMAKE_CXX_STANDARD 20)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/cmake-modules")

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs")
find_package(SDL2 REQUIRED)

set(SOURCES
    src/main.cpp
)

add_executable(chip8_emulator ${SOURCES})
target_link_libraries(chip8_emulator PRIVATE "${SDL2_LIBRARY}")
target_include_directories(chip8_emulator PRIVATE "${SDL2_INCLUDE_DIR}")