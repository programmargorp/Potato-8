cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 17)

project(potato-8)
set(EMU_VERSION_MAJOR 0)
set(EMU_VERSION_MINOR 1)
set(EMU_DEBUG_ENABLED 0)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

configure_file(
    "${PROJECT_SOURCE_DIR}/src/emu.h.in"
    "${PROJECT_BINARY_DIR}/src/build/emu.h"
)

include_directories("${PROJECT_BINARY_DIR}/src/build")

add_executable(${PROJECT_NAME} 
    src/c8.cpp
    src/mem.cpp
    src/cpu.cpp
    src/gfx.cpp
)

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2_LIBRARY})
