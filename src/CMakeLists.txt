file(GLOB_RECURSE LIB_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX "benchmark\\.cpp$")

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_executable(benchmark "${PROJECT_SOURCE_DIR}/src/benchmark.cpp")
    target_include_directories(benchmark PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
endif()
