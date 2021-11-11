add_subdirectory(glad)

file(GLOB D2RMH_SRC_FILES *.cpp *.h *.rc)
add_executable(D2RMH WIN32 ${D2RMH_SRC_FILES})
add_custom_target(D2RMH_RES DEPENDS D2RMH.ico)
add_dependencies(D2RMH D2RMH_RES)
target_include_directories(D2RMH PRIVATE ../d2mapapi stb)
target_link_libraries(D2RMH glad inih d2mapapi shlwapi winmm)
if(MSVC)
  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/lib/ntdll.lib
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ntdll.def
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/lib
    COMMAND lib /NOLOGO /DEF:${CMAKE_CURRENT_SOURCE_DIR}/ntdll.def /OUT:${CMAKE_BINARY_DIR}/lib/ntdll.lib /MACHINE:X86
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_BINARY_DIR}/lib/ntdll.exp)
  add_custom_target(ntdll_add DEPENDS ${CMAKE_BINARY_DIR}/lib/ntdll.lib)
  add_dependencies(D2RMH ntdll_add)
  target_link_libraries(D2RMH ${CMAKE_BINARY_DIR}/lib/ntdll.lib)
else()
  target_link_libraries(D2RMH ntdll)
endif()
set_target_properties(D2RMH PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
