cmake_minimum_required(VERSION 3.13)

add_subdirectory(simphttp)
add_library(d2mapapi STATIC
    collisionmap.cpp collisionmap.h
    d2map.cpp d2map.h
    d2ptrs.h d2structs.h
    offset.cpp offset.cpp
    session.cpp session.h)
target_include_directories(d2mapapi PUBLIC json)
set_target_properties(d2mapapi PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_executable(d2mapapi_httpd EXCLUDE_FROM_ALL httpd.cpp)
target_link_libraries(d2mapapi_httpd d2mapapi simphttp)
set_target_properties(d2mapapi_httpd PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(d2mapapi_piped EXCLUDE_FROM_ALL piped.cpp)
target_link_libraries(d2mapapi_piped d2mapapi)
set_target_properties(d2mapapi_piped PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(d2mapapi_host EXCLUDE_FROM_ALL host.cpp)
target_link_libraries(d2mapapi_host d2mapapi)
set_target_properties(d2mapapi_host PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
