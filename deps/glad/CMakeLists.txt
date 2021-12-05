add_library(glad STATIC EXCLUDE_FROM_ALL src/glad.c src/glad_wgl.c)
target_include_directories(glad PUBLIC include)
