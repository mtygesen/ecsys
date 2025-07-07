
file(GLOB_RECURSE LIB_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX "${PROJECT_NAME}\\.cpp$")

add_executable(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}.cpp")

if(LIB_SOURCES)
    add_library(lib${PROJECT_NAME} STATIC ${LIB_SOURCES})
    target_include_directories(lib${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
    set_target_properties(lib${PROJECT_NAME} PROPERTIES PREFIX "")
    
    if(MINGW)
        # bug with mingw so allow multiple defs: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=110572
        target_link_libraries(${PROJECT_NAME} PRIVATE lib${PROJECT_NAME} -static-libstdc++ -static-libgcc -Wl,--allow-multiple-definition)
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE lib${PROJECT_NAME} -static-libstdc++ -static-libgcc)
    endif()
endif()
