cmake_minimum_required(VERSION 3.16)
project(chip8-emulator)

set(CMAKE_CXX_STANDARD 20)


if (WIN32)
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs")
else()
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/cmake-modules")
endif ()
find_package(SDL2 REQUIRED)

set(SOURCES
        src/main.cpp
        src/utils.cpp)

add_executable(chip8-emulator ${SOURCES})
target_link_libraries(chip8-emulator PRIVATE "${SDL2_LIBRARY}")
target_include_directories(chip8-emulator PRIVATE "${SDL2_INCLUDE_DIR}")