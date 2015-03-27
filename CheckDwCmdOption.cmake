
# 检查更新CmdOption

if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/include/CmdOption" OR NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/src/CmdOption")
    find_package(Git REQUIRED)
    execute_process(COMMAND ${GIT_EXECUTABLE} clone "https://github.com/owt5008137/CmdOption.git"
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    )

    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/include/CmdOption" )
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/CmdOption")
    endif()

    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/src/CmdOption" )
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/src/CmdOption")
    endif()

    file(GLOB CMDOPTION_FILES "${CMAKE_CURRENT_LIST_DIR}/CmdOption/src/*.h")
    execute_process(COMMAND mv -f ${CMDOPTION_FILES} include/CmdOption
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    )

    file(GLOB CMDOPTION_FILES "${CMAKE_CURRENT_LIST_DIR}/CmdOption/src/*.cpp")
    execute_process(COMMAND mv -f ${CMDOPTION_FILES} src/CmdOption
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    )

    file(REMOVE_RECURSE "${CMAKE_CURRENT_LIST_DIR}/CmdOption")
endif()

include_directories("${CMAKE_CURRENT_LIST_DIR}/include/CmdOption")