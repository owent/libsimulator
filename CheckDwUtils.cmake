
# 检查更新owent-utils/c-cpp

if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/include/std" OR NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/include/ext")
    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/include/std" )
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/std")
    endif()

    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/include/ext" )
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/ext")
    endif()

    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/include/ext/Lock" )
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/ext/Lock")
    endif()

    set(DOWNLOAD_SOURCE_ADDR "https://raw.githubusercontent.com/owent-utils/c-cpp/master/include")
    set(DOWNLOAD_TARGET_ADDR "${CMAKE_CURRENT_LIST_DIR}/include/ext")

    macro(DOWN_UTILS_RUN SRC DST)
        message(STATUS "download file ${SRC} to ${DST}")
        file(DOWNLOAD
            "${SRC}"
            "${DST}"
            SHOW_PROGRESS
        )
    endmacro()

    message(STATUS "download ")
    DOWN_UTILS_RUN(
        "${DOWNLOAD_SOURCE_ADDR}/DesignPattern/Noncopyable.h"
        "${DOWNLOAD_TARGET_ADDR}/Noncopyable.h"
    )

    DOWN_UTILS_RUN(
        "${DOWNLOAD_SOURCE_ADDR}/DesignPattern/Singleton.h"
        "${DOWNLOAD_TARGET_ADDR}/Singleton.h"
    )

    DOWN_UTILS_RUN(
        "${DOWNLOAD_SOURCE_ADDR}/Lock/SpinLock.h"
        "${DOWNLOAD_TARGET_ADDR}/Lock/SpinLock.h"
    )

    set(DOWNLOAD_SOURCE_ADDR "https://raw.githubusercontent.com/owent-utils/c-cpp/master/include")
    set(DOWNLOAD_TARGET_ADDR "${CMAKE_CURRENT_LIST_DIR}/include")

    DOWN_UTILS_RUN(
        "${DOWNLOAD_SOURCE_ADDR}/std/array.h"
        "${DOWNLOAD_TARGET_ADDR}/std/array.h"
    )

    DOWN_UTILS_RUN(
         "${DOWNLOAD_SOURCE_ADDR}/std/explicit_declare.h"
         "${DOWNLOAD_TARGET_ADDR}/std/explicit_declare.h"
    )

    DOWN_UTILS_RUN(
         "${DOWNLOAD_SOURCE_ADDR}/std/functional.h"
         "${DOWNLOAD_TARGET_ADDR}/std/functional.h"
    )

    DOWN_UTILS_RUN(
         "${DOWNLOAD_SOURCE_ADDR}/std/ref.h"
         "${DOWNLOAD_TARGET_ADDR}/std/ref.h"
    )

    DOWN_UTILS_RUN(
         "${DOWNLOAD_SOURCE_ADDR}/std/smart_ptr.h"
         "${DOWNLOAD_TARGET_ADDR}/std/smart_ptr.h"
    )

    DOWN_UTILS_RUN(
         "${DOWNLOAD_SOURCE_ADDR}/std/static_assert.h"
         "${DOWNLOAD_TARGET_ADDR}/std/static_assert.h"
    )

    DOWN_UTILS_RUN(
         "${DOWNLOAD_SOURCE_ADDR}/std/thread.h"
         "${DOWNLOAD_TARGET_ADDR}/std/thread.h"
    )

    DOWN_UTILS_RUN(
         "${DOWNLOAD_SOURCE_ADDR}/std/tuple.h"
         "${DOWNLOAD_TARGET_ADDR}/std/tuple.h"
    )

    DOWN_UTILS_RUN(
         "${DOWNLOAD_SOURCE_ADDR}/std/utility.h"
         "${DOWNLOAD_TARGET_ADDR}/std/utility.h"
    )

    DOWN_UTILS_RUN(
         "${DOWNLOAD_SOURCE_ADDR}/std/foreach.h"
         "${DOWNLOAD_TARGET_ADDR}/std/foreach.h"
    )
endif()
