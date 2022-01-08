cmake_minimum_required(VERSION 3.13)
project(D2RMH C CXX)

if(MSVC)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 17)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(CustomCompilerOptions)
fix_compile_flags()
fix_release_flags()
add_static_runtime_option()

add_subdirectory(deps)
add_subdirectory(d2mapapi)
add_subdirectory(src)
