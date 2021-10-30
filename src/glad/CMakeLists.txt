add_library(glad STATIC src/glad.c src/glad_wgl.c)
target_include_directories(glad PUBLIC include)
