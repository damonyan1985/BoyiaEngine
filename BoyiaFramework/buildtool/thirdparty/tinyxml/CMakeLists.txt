cmake_minimum_required(VERSION 3.4.1)

project(tinyxml)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wno-unused-parameter")
set(TINYXML_PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../thirdparty/tinyxml")

if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
    include_directories(${ANDROID_SYSROOT}/usr/include/aarch64-linux-android)
else()
    include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
endif()

set(
    tinyxml_src
    "${TINYXML_PROJECT_PATH}/TinyXml2.cpp"
)

add_library(
    tinyxml
    SHARED
    ${tinyxml_src}
)

target_include_directories(
    tinyxml
    PRIVATE
    ${TINYXML_PROJECT_PATH}
    )