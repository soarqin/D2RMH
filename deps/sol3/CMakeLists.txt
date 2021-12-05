add_library(sol3 INTERFACE EXCLUDE_FROM_ALL)
target_include_directories(sol3 INTERFACE include)
target_link_libraries(sol3 INTERFACE lua::lua54)
