add_library(inih STATIC ini.c ini.h)
target_compile_definitions(inih PRIVATE INI_ALLOW_NO_VALUE INI_CALL_HANDLER_ON_NEW_SECTION)
target_include_directories(inih PUBLIC .)
set_target_properties(inih PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
