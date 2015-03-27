
include_directories (${CMAKE_CURRENT_LIST_DIR})
include_directories (${CMAKE_CURRENT_LIST_DIR}/../include)

link_directories (${LIBRARY_OUTPUT_PATH})

file(GLOB_RECURSE SRC_LIST *.h *.hpp *.c *.cpp)
source_group_by_dir(SRC_LIST)

if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
	add_definitions(-D_POSIX_MT_ -pthread)
endif()

add_executable(simple_test  ${SRC_LIST})

target_link_libraries(simple_test
    simulator history readline termcap
    m dl rt pthread
)

