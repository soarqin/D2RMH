add_subdirectory(CascLib)

add_executable(gendata gendata.cpp)
target_compile_definitions(gendata PRIVATE CASCLIB_NO_AUTO_LINK_LIBRARY)
target_include_directories(gendata PRIVATE ../json)
target_link_libraries(gendata inih common casc)
set_target_properties(gendata PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
