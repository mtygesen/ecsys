cmake_minimum_required(VERSION 3.28)

find_program(GCC13 "gcc-13")
find_program(GPP13 "g++-13")

if(GCC13 AND GPP13)
    set(CMAKE_C_COMPILER "${GCC13}")
    set(CMAKE_CXX_COMPILER "${GPP13}")
endif()

project(ecsys)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Macros
add_definitions(-DSHADER_ROOT="${CMAKE_SOURCE_DIR}/src/shaders")

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/W4 /Od /DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Wpedantic -Wconversion -Wsign-conversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_UPPER)
message(STATUS "${CMAKE_BUILD_TYPE} CXX Flags: ${CMAKE_CXX_FLAGS_${BUILD_TYPE_UPPER}}")
message(STATUS "${CMAKE_BUILD_TYPE} Linker Flags: ${CMAKE_EXE_LINKER_FLAGS_${BUILD_TYPE_UPPER}}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)