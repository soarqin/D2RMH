add_subdirectory(CascLib)

add_executable(gendata gendata.cpp)
target_include_directories(gendata PRIVATE ../json)
target_link_libraries(gendata inih common casc)
