project(llhttp)
add_library(${PROJECT_NAME} STATIC EXCLUDE_FROM_ALL api.c api.h http.c llhttp.c llhttp.h)
target_include_directories(${PROJECT_NAME} PUBLIC .)
