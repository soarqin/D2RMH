include(GetVersion)

option(USE_FREETYPE "Use FreeType to render TTF fonts" OFF)
file(GLOB D2RMH_SRC_FILES
    d2r/*.cpp d2r/*.h
    data/*.cpp data/*.h
    plugin/*.cpp plugin/*.h
    render/*.cpp render/*.h
    ui/*.cpp ui/*.h
    util/*.cpp util/*.h
    *.cpp *.h
    res/res.rc)
if(NOT MSVC)
  set(D2RMH_SRC_FILES ${D2RMH_SRC_FILES} res/manifest.rc)
endif()
add_executable(D2RMH WIN32 ${D2RMH_SRC_FILES})
get_project_version(D2RMH)
add_custom_target(D2RMH_RES DEPENDS res/D2RMH.ico)
add_dependencies(D2RMH D2RMH_RES)
if(TARGET d2mapapi_piped)
  add_dependencies(D2RMH d2mapapi_piped)
endif()
if(USE_FREETYPE)
  find_package(Freetype REQUIRED)
  target_compile_definitions(D2RMH PRIVATE USE_FREETYPE)
  target_link_libraries(D2RMH Freetype::Freetype)
endif()
target_include_directories(D2RMH PRIVATE .)
target_link_libraries(D2RMH casc glad stb inih sol3 d2mapapi_pipehost dwmapi shlwapi comctl32 winmm)
set_target_properties(D2RMH PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
