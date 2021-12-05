macro(get_project_version VER_PROJ_NAME)
    find_package(Git QUIET)

    # Check if git is found...
    if (GIT_FOUND)

        # Get last tag from git
        execute_process(COMMAND ${GIT_EXECUTABLE} describe --abbrev=0 --tags
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE ${VER_PROJ_NAME}_VERSION_STRING
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        # Get name of current branch
        execute_process(COMMAND ${GIT_EXECUTABLE} symbolic-ref --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE ${VER_PROJ_NAME}_BRANCH_NAME
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        #How many commits since last tag
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-list ${${VER_PROJ_NAME}_BRANCH_NAME} ${${VER_PROJ_NAME}_VERSION_STRING}..HEAD --count
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE ${VER_PROJ_NAME}_VERSION_AHEAD
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        # Get current commit SHA from git
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE ${VER_PROJ_NAME}_VERSION_GIT_SHA
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        # Get partial versions into a list
        string(REGEX MATCHALL "-.*$|[0-9]+" ${VER_PROJ_NAME}_PARTIAL_VERSION_LIST
            ${${VER_PROJ_NAME}_VERSION_STRING})

        # Set the version numbers
        list(GET ${VER_PROJ_NAME}_PARTIAL_VERSION_LIST
            0 ${VER_PROJ_NAME}_VERSION_MAJOR)
        list(GET ${VER_PROJ_NAME}_PARTIAL_VERSION_LIST
            1 ${VER_PROJ_NAME}_VERSION_MINOR)
        list(GET ${VER_PROJ_NAME}_PARTIAL_VERSION_LIST
            2 ${VER_PROJ_NAME}_VERSION_PATCH)

        # The tweak part is optional, so check if the list contains it
        list(LENGTH ${VER_PROJ_NAME}_PARTIAL_VERSION_LIST
            ${VER_PROJ_NAME}_PARTIAL_VERSION_LIST_LEN)
        if (${VER_PROJ_NAME}_PARTIAL_VERSION_LIST_LEN GREATER 3)
            list(GET ${VER_PROJ_NAME}_PARTIAL_VERSION_LIST 3 ${VER_PROJ_NAME}_VERSION_TWEAK)
            string(SUBSTRING ${${VER_PROJ_NAME}_VERSION_TWEAK} 1 -1 ${VER_PROJ_NAME}_VERSION_TWEAK)
        endif()

        # Unset the list
        unset(${VER_PROJ_NAME}_PARTIAL_VERSION_LIST)

        # Save version to file (which will be used when Git is not available
        # or VERSION_UPDATE_FROM_GIT is disabled)
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/version_generated ${${VER_PROJ_NAME}_VERSION_STRING}
            "*" ${${VER_PROJ_NAME}_VERSION_MAJOR}
            "*" ${${VER_PROJ_NAME}_VERSION_MINOR}
            "*" ${${VER_PROJ_NAME}_VERSION_PATCH}
            "*" ${${VER_PROJ_NAME}_VERSION_TWEAK}
            "*" ${${VER_PROJ_NAME}_VERSION_AHEAD}
            "*" ${${VER_PROJ_NAME}_VERSION_GIT_SHA})

    else()

        # Git not available, get version from file
        file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/version_generated ${VER_PROJ_NAME}_VERSION_LIST)
        string(REPLACE "*" ";" ${VER_PROJ_NAME}_VERSION_LIST "${${VER_PROJ_NAME}_VERSION_LIST}")
        # Set partial versions
        list(GET ${VER_PROJ_NAME}_VERSION_LIST 0 ${VER_PROJ_NAME}_VERSION_STRING)
        list(GET ${VER_PROJ_NAME}_VERSION_LIST 1 ${VER_PROJ_NAME}_VERSION_MAJOR)
        list(GET ${VER_PROJ_NAME}_VERSION_LIST 2 ${VER_PROJ_NAME}_VERSION_MINOR)
        list(GET ${VER_PROJ_NAME}_VERSION_LIST 3 ${VER_PROJ_NAME}_VERSION_PATCH)
        list(GET ${VER_PROJ_NAME}_VERSION_LIST 4 ${VER_PROJ_NAME}_VERSION_TWEAK)
        list(GET ${VER_PROJ_NAME}_VERSION_LIST 5 ${VER_PROJ_NAME}_VERSION_AHEAD)
        list(GET ${VER_PROJ_NAME}_VERSION_LIST 6 ${VER_PROJ_NAME}_VERSION_GIT_SHA)

    endif()

    # Set full project version string
    if (${VER_PROJ_NAME}_VERSION_AHEAD GREATER 0)
        set(${VER_PROJ_NAME}_VERSION_STRING_FULL
            ${${VER_PROJ_NAME}_VERSION_STRING}+${${VER_PROJ_NAME}_VERSION_AHEAD}.${${VER_PROJ_NAME}_VERSION_GIT_SHA})
    else()
        set(${VER_PROJ_NAME}_VERSION_STRING_FULL
            ${${VER_PROJ_NAME}_VERSION_STRING}.${${VER_PROJ_NAME}_VERSION_GIT_SHA})
    endif()

    # Set project version (without the preceding 'v')
    set(${VER_PROJ_NAME}_VERSION ${${VER_PROJ_NAME}_VERSION_MAJOR}.${${VER_PROJ_NAME}_VERSION_MINOR}.${${VER_PROJ_NAME}_VERSION_PATCH})
    if (${VER_PROJ_NAME}_VERSION_TWEAK)
        set(${VER_PROJ_NAME}_VERSION ${${VER_PROJ_NAME}_VERSION}-${${VER_PROJ_NAME}_VERSION_TWEAK})
    endif()

    target_compile_definitions(${VER_PROJ_NAME} PRIVATE
        VERSION_STRING_FULL="${${VER_PROJ_NAME}_VERSION_STRING_FULL}"
        VERSION_STRING="${${VER_PROJ_NAME}_VERSION_STRING}"
        VERSION_MAJOR=${${VER_PROJ_NAME}_VERSION_MAJOR}
        VERSION_MINOR=${${VER_PROJ_NAME}_VERSION_MINOR}
        VERSION_PATCH=${${VER_PROJ_NAME}_VERSION_PATCH}
        VERSION_TWEAK="${${VER_PROJ_NAME}_VERSION_TWEAK}"
        VERSION_AHEAD="${${VER_PROJ_NAME}_VERSION_AHEAD}"
        VERSION_GIT_SHA="${${VER_PROJ_NAME}_VERSION_GIT_SHA}"
        )
endmacro()
