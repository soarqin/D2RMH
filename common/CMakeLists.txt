add_library(common STATIC d2txt.cpp d2txt.h jsonlng.cpp jsonlng.h viewstream.h)
target_include_directories(common PUBLIC . PRIVATE ../json)
