cmake_minimum_required(VERSION 3.13)

project(simphttp)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(Git QUIET REQUIRED)
include(ExternalProject)
find_package(LibUV QUIET)
if(LIBUV_FOUND)
    message(STATUS "Using system libuv")
    add_library(libuv_external INTERFACE)
    target_include_directories(libuv_external INTERFACE ${LIBUV_INCLUDE_DIRS})
    target_link_libraries(libuv_external INTERFACE ${LIBUV_LIBRARIES})
    add_library(uv::libuv ALIAS libuv_external)
else()
    ExternalProject_Add(libuv
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libuv
        STEP_TARGETS update
        GIT_REPOSITORY https://github.com/libuv/libuv.git
        GIT_TAG v1.x
        GIT_SHALLOW ON
        UPDATE_DISCONNECTED ON
        STEP_TARGETS update
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                   -DCMAKE_INSTALL_LIBDIR=lib -DLIBUV_BUILD_TESTS=OFF -DLIBUV_BUILD_BENCH=OFF
        )
    add_library(uv-static INTERFACE)
    add_dependencies(uv-static libuv)
    target_include_directories(uv-static INTERFACE ${CMAKE_BINARY_DIR}/include)
    target_link_directories(uv-static INTERFACE ${CMAKE_BINARY_DIR}/lib)
    find_package(Threads)
    target_link_libraries(uv-static INTERFACE uv_a Threads::Threads ${CMAKE_DL_LIBS})
    if(WIN32)
        target_link_libraries(uv-static INTERFACE ws2_32 iphlpapi userenv)
    endif()
    add_library(uv::libuv ALIAS uv-static)
endif()

add_subdirectory(llhttp)

add_library(${PROJECT_NAME} STATIC EXCLUDE_FROM_ALL simphttp.cpp simphttp.h)
target_include_directories(${PROJECT_NAME} PUBLIC .)
target_link_libraries(${PROJECT_NAME} llhttp uv::libuv)
