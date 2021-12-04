cmake_minimum_required(VERSION 3.13)

project(d2mapapi)

set(VERSION_MAJOR)

if(MSVC)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 17)
endif()

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
    list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
    include(CustomCompilerOptions)
    fix_release_flags()
    add_static_runtime_option()
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_subdirectory(simphttp)
    add_library(d2mapapi STATIC EXCLUDE_FROM_ALL
        collisionmap.cpp collisionmap.h
        mapdata.cpp mapdata.h
        d2map.cpp d2map.h
        d2ptrs.h d2structs.h
        offset.cpp offset.cpp
        session.cpp session.h)
    target_include_directories(d2mapapi PUBLIC . PRIVATE json)
    set_target_properties(d2mapapi PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

    add_executable(d2mapapi_httpd EXCLUDE_FROM_ALL httpd.cpp)
    target_link_libraries(d2mapapi_httpd d2mapapi simphttp)
    if(MSVC)
        target_link_options(d2mapapi_httpd PRIVATE /ENTRY:wmainCRTStartup)
    else()
        target_link_options(d2mapapi_httpd PRIVATE -municode)
    endif()
    set_target_properties(d2mapapi_httpd PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    add_executable(d2mapapi_piped WIN32 EXCLUDE_FROM_ALL piped.cpp)
    target_link_libraries(d2mapapi_piped d2mapapi)
    if(MSVC)
        target_link_options(d2mapapi_piped PRIVATE /ENTRY:wmainCRTStartup)
    else()
        target_link_options(d2mapapi_piped PRIVATE -municode)
    endif()
    set_target_properties(d2mapapi_piped PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

add_library(d2mapapi_pipehost STATIC EXCLUDE_FROM_ALL
    collisionmap.cpp collisionmap.h
    pipehost.cpp pipehost.h)
target_include_directories(d2mapapi_pipehost PUBLIC . PRIVATE json)
set_target_properties(d2mapapi_pipehost PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# this is a test program for d2mapapi_pipehost functions
add_executable(d2mapapi_host EXCLUDE_FROM_ALL host.cpp)
target_link_libraries(d2mapapi_host d2mapapi_pipehost)
set_target_properties(d2mapapi_host PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
