cmake_minimum_required(VERSION 3.4.1)

project(curl)

add_definitions(-DHAVE_CONFIG_H -DBUILDING_LIBCURL)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wno-unused-parameter -Wpointer-arith -Wwrite-strings -Wunused -Winline -Wnested-externs -Wmissing-declarations -Wmissing-prototypes -Wno-long-long -Wfloat-equal -Wno-multichar -Wsign-compare -Wno-format-nonliteral -Wendif-labels -Wstrict-prototypes -Wdeclaration-after-statemen -Wno-system-headers")

#set(CMAKE_C_FLAGS "-Wpointer-arith -Wwrite-strings -Wunused -Winline -Wnested-externs -Wmissing-declarations -Wmissing-prototypes -Wno-long-long -Wfloat-equal -Wno-multichar -Wsign-compare -Wno-format-nonliteral -Wendif-labels -Wstrict-prototypes -Wdeclaration-after-statemen -Wno-system-headers")

set(CURL_PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

file(
    GLOB 
    curl_src
    "lib/*.c"
)

file(
    GLOB 
    vtls_src
    "lib/vtls/*.c"
)

file(
    GLOB 
    vauth_src
    "lib/vauth/*.c"
)

add_library(
    curl
    SHARED
    ${curl_src}
    ${vtls_src}
    ${vauth_src}
)

target_link_libraries(
    curl
    ssl
    crypto
)

target_include_directories(
    curl
    PRIVATE
    ${CURL_PROJECT_PATH}/include
    ${CURL_PROJECT_PATH}/lib
    ${CURL_PROJECT_PATH}/../ssl/include
    ${CURL_PROJECT_PATH}/../ssl/crypto
    )