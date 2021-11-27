cmake_minimum_required(VERSION 3.13)
project(D2RMH C CXX)

if(MSVC)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 17)
endif()


option(USE_STATIC_CRT "Use static C runtime" OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")
endif()

if(USE_STATIC_CRT)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -flto")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
    elseif(MSVC)
        set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELWITHDEBINFO
            )
        foreach(CompilerFlag ${CompilerFlags})
            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
            set(${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "msvc compiler flags" FORCE)
        endforeach()
        add_compile_options(
            $<$<CONFIG:>:/MT>
            $<$<CONFIG:Debug>:/MTd>
            $<$<CONFIG:Release>:/MT>
            $<$<CONFIG:MinSizeRel>:/MT>
            $<$<CONFIG:RelWithDebInfo>:/MT>
        )
    endif()
endif()
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    add_compile_options(
        $<$<CONFIG:Release>:/GL>
        $<$<CONFIG:MinSizeRel>:/GL>
    )
    add_link_options(
        $<$<CONFIG:Release>:/LTCG>
        $<$<CONFIG:MinSizeRel>:/LTCG>
    )
endif()

add_subdirectory(common)
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
