cmake_minimum_required(VERSION 3.4.1)

project(cjson)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wno-unused-parameter")
set(CJSON_PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../source/thirdparty/cjson")

set(
    cjson_src
    "${CJSON_PROJECT_PATH}/cJSON.c"
)

add_library(
    cjson
    SHARED
    ${cjson_src}
)

target_include_directories(
    cjson
    PRIVATE
    ${CJSON_PROJECT_PATH}
    )