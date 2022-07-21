cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Astro8-Emulator.exe)

SET(programName Astro8-Emulator)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeFiles/")

add_executable (Astro8-Emulator.exe main.cpp color.hpp colorprint.h)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

target_link_libraries(Astro8-Emulator.exe m ${SDL2_LIBRARIES})
