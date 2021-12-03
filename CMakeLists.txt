cmake_minimum_required(VERSION 3.13)
project(D2RMH C CXX)

if(MSVC)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 17)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(CustomCompilerOptions)
fix_release_flags()
add_static_runtime_option()

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()
add_subdirectory(inih)
add_subdirectory(d2mapapi)
add_subdirectory(src)
add_subdirectory(tools)

set(ZIP_OUTPUT "${CMAKE_BINARY_DIR}/bin/D2RMH-snapshot.zip")
add_custom_command(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:D2RMH> ${CMAKE_SOURCE_DIR}/bin/D2RMH.ini ${CMAKE_SOURCE_DIR}/bin/D2RMH_data.ini ${CMAKE_SOURCE_DIR}/bin/D2RMH_item.ini ${CMAKE_SOURCE_DIR}/README.md ${CMAKE_SOURCE_DIR}/ChangeLog.md ${CMAKE_SOURCE_DIR}/TODO.md ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E tar "cf" "${ZIP_OUTPUT}" --format=zip -- D2RMH.exe D2RMH.ini D2RMH_data.ini D2RMH_item.ini README.md ChangeLog.md TODO.md LICENSE
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT  "${ZIP_OUTPUT}"
    DEPENDS D2RMH
    COMMENT "Zipping to ${ZIP_OUTPUT}."
)
set(ZIP_OUTPUT2 "${CMAKE_BINARY_DIR}/bin/gendata-snapshot.zip")
add_custom_command(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:gendata> ${CMAKE_SOURCE_DIR}/bin/gendata.ini ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E tar "cf" "${ZIP_OUTPUT2}" --format=zip -- gendata.exe gendata.ini
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT  "${ZIP_OUTPUT2}"
    DEPENDS gendata
    COMMENT "Zipping to ${ZIP_OUTPUT2}."
)
add_custom_target(snapshot-pack
    DEPENDS "${ZIP_OUTPUT}" "${ZIP_OUTPUT2}")
