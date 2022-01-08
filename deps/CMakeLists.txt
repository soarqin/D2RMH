add_subdirectory(glad)
add_subdirectory(stb)
add_subdirectory(inih)
add_subdirectory(CascLib)

find_package(Git REQUIRED QUIET)
include(ExternalProject)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LUA54_LIB_SUFFIX "d")
endif()
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(LUA54_INSTALL_COMMANDS
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <BINARY_DIR>/bin64 ${CMAKE_BINARY_DIR}/bin
                COMMAND ${CMAKE_COMMAND} -E copy_directory <BINARY_DIR>/include ${CMAKE_BINARY_DIR}/include/lua
                COMMAND ${CMAKE_COMMAND} -E copy_directory <BINARY_DIR>/lib64 ${CMAKE_BINARY_DIR}/lib
        )
else()
    set(LUA54_INSTALL_COMMANDS
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <BINARY_DIR>/bin ${CMAKE_BINARY_DIR}/bin
                COMMAND ${CMAKE_COMMAND} -E copy_directory <BINARY_DIR>/include ${CMAKE_BINARY_DIR}/include/lua
                COMMAND ${CMAKE_COMMAND} -E copy_directory <BINARY_DIR>/lib ${CMAKE_BINARY_DIR}/lib
        )
endif()
ExternalProject_Add(lua54
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/lua54
    GIT_REPOSITORY https://github.com/NLua/lua.git
    GIT_TAG main
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL ON
    UPDATE_DISCONNECTED ON
    STEP_TARGETS update
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR} -DLUA_BUILD_AS_DLL=OFF
    ${LUA54_INSTALL_COMMANDS}
    )
add_library(lua54-static INTERFACE)
add_dependencies(lua54-static lua54)
target_include_directories(lua54-static INTERFACE ${CMAKE_BINARY_DIR}/include/lua)
target_link_directories(lua54-static INTERFACE ${CMAKE_BINARY_DIR}/lib)
target_link_libraries(lua54-static INTERFACE lua54_static${LUA54_LIB_SUFFIX})
add_library(lua::lua54 ALIAS lua54-static)
list(APPEND EXTERNAL_PROJECTS lua54)

add_subdirectory(sol3)

# update external project
add_custom_command(OUTPUT ${PROJECT_NAME}-reset_projects COMMAND "")
add_custom_target(${PROJECT_NAME}-update_deps DEPENDS simpio-update_deps ${PROJECT_NAME}-reset_projects)
foreach(PROJ ${EXTERNAL_PROJECTS})
    add_dependencies(${PROJECT_NAME}-update_deps ${PROJ}-update)
    ExternalProject_Get_property(${PROJ} STAMP_DIR)
    add_custom_command(OUTPUT ${PROJECT_NAME}-reset_projects
        COMMAND ${CMAKE_COMMAND} -E rm -f ${STAMP_DIR}/${PROJ}-patch ${STAMP_DIR}/${PROJ}-configure ${STAMP_DIR}/${PROJ}-build ${STAMP_DIR}/${PROJ}-install ${STAMP_DIR}/${PROJ}-done
        APPEND)
endforeach()
set_source_files_properties(${PROJECT_NAME}-reset_projects PROPERTIES SYMBOLIC "true")
