cmake_minimum_required(VERSION 3.4.1)

project(zip)

add_definitions(-DIOAPI_NO_64 -DENABLE_BOYIA_ANDROID=1)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wno-unused-parameter")
set(ZIP_PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../source/thirdparty/zip")

set(ZLIB_SRCS
    ${ZIP_PROJECT_PATH}/adler32.c
    ${ZIP_PROJECT_PATH}/compress.c
    ${ZIP_PROJECT_PATH}/crc32.c
    ${ZIP_PROJECT_PATH}/deflate.c
    ${ZIP_PROJECT_PATH}/gzclose.c
    ${ZIP_PROJECT_PATH}/gzlib.c
    ${ZIP_PROJECT_PATH}/gzread.c
    ${ZIP_PROJECT_PATH}/gzwrite.c
    ${ZIP_PROJECT_PATH}/inflate.c
    ${ZIP_PROJECT_PATH}/infback.c
    ${ZIP_PROJECT_PATH}/inftrees.c
    ${ZIP_PROJECT_PATH}/inffast.c
    ${ZIP_PROJECT_PATH}/trees.c
    ${ZIP_PROJECT_PATH}/uncompr.c
    ${ZIP_PROJECT_PATH}/zutil.c
)

set(ZIP_SRC
    ${ZIP_PROJECT_PATH}/minizip/unzip.c
    ${ZIP_PROJECT_PATH}/minizip/ioapi.c
)

add_library(
    zip
    SHARED
    ${ZLIB_SRCS}
    ${ZIP_SRC}
)

target_include_directories(
    zip
    PRIVATE
    ${ZIP_PROJECT_PATH}
    ${ZIP_PROJECT_PATH}/minizip
    )
