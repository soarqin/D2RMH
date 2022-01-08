add_executable(gendata EXCLUDE_FROM_ALL
    gendata.cpp
    d2txt.cpp d2txt.h
    jsonlng.cpp jsonlng.h
    viewstream.h)
target_include_directories(gendata PRIVATE ../d2mapapi/json)
target_link_libraries(gendata inih casc)
set_target_properties(gendata PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
