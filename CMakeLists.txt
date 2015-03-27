
cmake_minimum_required(VERSION 2.8.9)
project(libsimulator)


set (PROJECT_INC_DIR "${CMAKE_CURRENT_LIST_DIR}/include")
set (PROJECT_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set (PROJECT_LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/lib")
set (PROJECT_LIB_NAME "simulator")

########################################################################
# CMake 模块 (递归包含模块, 带颜色输出模块, 平台检测模块)
set (PROJECT_CMAKE_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}/project/cmake")
include("${PROJECT_CMAKE_MODULE_DIR}/IncludeDirectoryRecurse.cmake")
include("${PROJECT_CMAKE_MODULE_DIR}/EchoWithColor.cmake")
include("${PROJECT_CMAKE_MODULE_DIR}/FindPlatform.cmake")
include("${PROJECT_CMAKE_MODULE_DIR}/ProjectTools.cmake")
include("${PROJECT_CMAKE_MODULE_DIR}/CompilerOption.cmake")
include("${PROJECT_CMAKE_MODULE_DIR}/TargetOption.cmake")

# 系统依赖库检测
set(TERMCAP_LIB_NAME "termcap")
find_library(TERMCAP_LIB_NAME_EXISTS ${TERMCAP_LIB_NAME})
if (TERMCAP_LIB_NAME_EXISTS)
	EchoWithColor(COLOR GREEN "-- Dep [${TERMCAP_LIB_NAME}] Found: ${TERMCAP_LIB_NAME_EXISTS}")
elseif(NOT WIN32 AND NOT CYGWIN)
	message(FATAL_ERROR "${TERMCAP_LIB_NAME} not found")
else()
	message(STATUS "Now in win32 platform, skip ${TERMCAP_LIB_NAME} check.")
endif()

set(READLINE_ROOT "" CACHE STRING "GNU readline root directory")

# 如果指定了readline库目录，优先选择
if (READLINE_ROOT)
    include_directories ("${READLINE_ROOT}/include")
    link_directories ("${READLINE_ROOT}/lib")
    link_directories ("${READLINE_ROOT}/lib64")
endif()
include_directories ("${PROJECT_INC_DIR}")
link_directories ("${PROJECT_LIB_DIR}")

# 自动下载依赖文件
include("${CMAKE_CURRENT_LIST_DIR}/CheckDwCmdOption.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/CheckDwUtils.cmake")


file(GLOB_RECURSE SRC_LIST ${PROJECT_INC_DIR}/*.h ${PROJECT_INC_DIR}/*.hpp ${PROJECT_SRC_DIR}/*.c ${PROJECT_SRC_DIR}/*.cc ${PROJECT_SRC_DIR}/*.cpp)
source_group_by_dir(SRC_LIST)


if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
	add_definitions(-D_POSIX_MT_ -pthread)
endif() 


add_library(${PROJECT_LIB_NAME} STATIC ${SRC_LIST})

# 安装目标
install(TARGETS ${PROJECT_LIB_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY "${PROJECT_INC_DIR}"
    DESTINATION .
    USE_SOURCE_PERMISSIONS
    PATTERN ".svn" EXCLUDE
    PATTERN ".git" EXCLUDE
)

add_subdirectory(test)
