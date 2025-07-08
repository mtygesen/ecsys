
file(GLOB_RECURSE LIB_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX "${PROJECT_NAME}\\.cpp$")

add_executable(benchmark "${PROJECT_SOURCE_DIR}/src/benchmark.cpp")
target_include_directories(benchmark PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(LIB_SOURCES)
    add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    
    if(MINGW)
        # bug with mingw so allow multiple defs: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=110572
        target_link_libraries(benchmark PRIVATE ${PROJECT_NAME} -static-libstdc++ -static-libgcc -Wl,--allow-multiple-definition)
    else()
        target_link_libraries(benchmark PRIVATE ${PROJECT_NAME} -static-libstdc++ -static-libgcc)
    endif()
endif()
