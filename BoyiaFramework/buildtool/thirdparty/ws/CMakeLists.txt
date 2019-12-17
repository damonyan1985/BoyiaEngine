cmake_minimum_required(VERSION 3.4.1)

project(ws)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wno-unused-parameter")
set(WSOCKET_PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../source/thirdparty/ws")

if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
    include_directories(${ANDROID_SYSROOT}/usr/include/aarch64-linux-android)
else()
    include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
endif()

set(
    ws_src
    "${WSOCKET_PROJECT_PATH}/WebSocket.cpp"
)

add_library(
    ws
    SHARED
    ${ws_src}
)

target_include_directories(
    ws
    PRIVATE
    ${WSOCKET_PROJECT_PATH}
    )