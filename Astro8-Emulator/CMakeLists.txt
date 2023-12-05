cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

project(astro8)

SET(programName astro8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")

file(GLOB VSSynth_SRC
     "VSSynth/*.h"
     "VSSynth/*.cpp"
     "VSSynth/utils/*.h"
     "VSSynth/utils/*.cpp"
     "VSSynth/middleware/*.h"
     "VSSynth/middleware/*.cpp"
     "VSSynth/generators/*.h"
     "VSSynth/generators/*.cpp"
)

add_executable (astro8 main.cpp color.hpp colorprint.h processing.h armstrong-compiler.h ${VSSynth_SRC})

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})

target_link_libraries(astro8 m ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARIES} ${CMAKE_CURRENT_SOURCE_DIR}/libVSSynth.a -lpthread)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../char_set_memtape
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})