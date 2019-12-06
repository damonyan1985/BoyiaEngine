cmake_minimum_required(VERSION 3.4.1)

project(boyia)

add_definitions(-DANDROID_NDK -DFT2_BUILD_LIBRARY=1 -DPOSIX -DENABLE_BOYIA_ANDROID=1)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -frtti")

set(BOYIA_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../source/boyia")
set(THIRD_PARTY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../source/thirdparty")

if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
    include_directories(${ANDROID_SYSROOT}/usr/include/aarch64-linux-android)
else()
    include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
endif()

file(
    GLOB 
    kernal_src
    "${BOYIA_SRC_PATH}/kernal/vm/view/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/vm/core/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/vm/jit/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/vm/lib/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/vm/json/*.c"
    "${BOYIA_SRC_PATH}/kernal/vm/json/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/dom/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/gui/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/loader/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/platform/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/platform/android/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/platform/arm/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/platform/graphics/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/platform/zip/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/style/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/util/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/view/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/widget/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/widget/flex/*.cpp"
    "${BOYIA_SRC_PATH}/kernal/app/*.cpp"
    )

file(
    GLOB 
    painter_src
    "${BOYIA_SRC_PATH}/painter/core/*.cpp"
    "${BOYIA_SRC_PATH}/painter/painter/*.cpp"
    "${BOYIA_SRC_PATH}/painter/shaders/*.cpp"
    "${BOYIA_SRC_PATH}/painter/utils/*.cpp"
    "${BOYIA_SRC_PATH}/painter/vulkan/*.cpp"
    ) 

file(
    GLOB 
    thread_src
    "${BOYIA_SRC_PATH}/thread/*.cpp"
    )

file(
    GLOB 
    porting_src
    "${BOYIA_SRC_PATH}/porting/android/*.cpp"
    )          

add_library(
    boyia
    SHARED
    ${kernal_src}
    ${painter_src}
    ${thread_src}
    ${porting_src}
)

target_include_directories(
    boyia
    PRIVATE
    ${BOYIA_SRC_PATH}/kernal/vm/view
    ${BOYIA_SRC_PATH}/kernal/vm/core
    ${BOYIA_SRC_PATH}/kernal/vm/jit
    ${BOYIA_SRC_PATH}/kernal/vm/json
    ${BOYIA_SRC_PATH}/kernal/vm/lib
    ${BOYIA_SRC_PATH}/kernal/app
    ${BOYIA_SRC_PATH}/kernal/dom
    ${BOYIA_SRC_PATH}/kernal/gui
    ${BOYIA_SRC_PATH}/kernal/loader
    ${BOYIA_SRC_PATH}/kernal/platform/android
    ${BOYIA_SRC_PATH}/kernal/platform/arm
    ${BOYIA_SRC_PATH}/kernal/platform/graphics
    ${BOYIA_SRC_PATH}/kernal/platform/zip
    ${BOYIA_SRC_PATH}/kernal/platform
    ${BOYIA_SRC_PATH}/kernal/style
    ${BOYIA_SRC_PATH}/kernal/util
    ${BOYIA_SRC_PATH}/kernal/view
    ${BOYIA_SRC_PATH}/kernal/widget
    ${BOYIA_SRC_PATH}/kernal/widget/flex
    ${BOYIA_SRC_PATH}/painter/core
    ${BOYIA_SRC_PATH}/painter/shaders
    ${BOYIA_SRC_PATH}/painter/painter
    ${BOYIA_SRC_PATH}/painter/utils
    ${BOYIA_SRC_PATH}/painter/vulkan
    ${BOYIA_SRC_PATH}/thread
    ${THIRD_PARTY_PATH}/cjson
    ${THIRD_PARTY_PATH}/tinyxml
    ${THIRD_PARTY_PATH}/zip
    ${THIRD_PARTY_PATH}/zip/minizip
)

find_library(
    boyia_link_lib
    log
    android
)

target_link_libraries(
    boyia
    log
    EGL
    GLESv3
    android
    jnigraphics
    cjson
    tinyxml
    zip
)