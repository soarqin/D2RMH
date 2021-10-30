add_subdirectory(glad)

file(GLOB D2RMH_SRC_FILES *.cpp *.h *.rc)
add_executable(D2RMH WIN32 ${D2RMH_SRC_FILES})
target_include_directories(D2RMH PRIVATE ../d2mapapi stb)
target_link_libraries(D2RMH glad inih d2mapapi shlwapi)
set_target_properties(D2RMH PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
