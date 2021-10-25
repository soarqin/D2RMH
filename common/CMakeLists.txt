add_library(common STATIC d2txt.cpp d2txt.h jsonlng.cpp jsonlng.h viewstream.h)
target_include_directories(common PUBLIC . PRIVATE ../json)
set_target_properties(common PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
