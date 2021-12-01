file(GLOB SRC_FILES *.cpp *.h)
add_library(d2mapapi STATIC ${SRC_FILES})
set_target_properties(d2mapapi PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
target_include_directories(d2mapapi PRIVATE ../json)
