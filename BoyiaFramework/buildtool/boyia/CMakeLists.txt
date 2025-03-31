cmake_minimum_required(VERSION 3.4.1)

project(boyia)

add_definitions(-DANDROID_NDK -DFT2_BUILD_LIBRARY=1 -DPOSIX -DENABLE_BOYIA_ANDROID=1 -DENABLE_BOYIA_PLATFORM_VIEW=1 -DBOYIA_UI_VSYNC=1)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -frtti")

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(BOYIA_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../source/boyia")
set(THIRD_PARTY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../source/thirdparty")
set(BOYIA_KERNEL_PATH "${BOYIA_SRC_PATH}/kernel")

if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
    include_directories(${ANDROID_SYSROOT}/usr/include/aarch64-linux-android)
else()
    include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
endif()

file(
    GLOB 
    kernel_src
    "${BOYIA_KERNEL_PATH}/vm/view/*.cpp"
    "${BOYIA_KERNEL_PATH}/vm/core/*.cpp"
    "${BOYIA_KERNEL_PATH}/vm/jit/*.cpp"
    "${BOYIA_KERNEL_PATH}/vm/lib/*.cpp"
    "${BOYIA_KERNEL_PATH}/vm/dbg/*.cpp"
    "${BOYIA_KERNEL_PATH}/vm/json/*.c"
    "${BOYIA_KERNEL_PATH}/vm/json/*.cpp"
    "${BOYIA_KERNEL_PATH}/dom/*.cpp"
    "${BOYIA_KERNEL_PATH}/gui/*.cpp"
    "${BOYIA_KERNEL_PATH}/loader/*.cpp"
    "${BOYIA_KERNEL_PATH}/render/*.cpp"
    "${BOYIA_KERNEL_PATH}/render/layer/*.cpp"
    "${BOYIA_KERNEL_PATH}/platform/*.cpp"
    "${BOYIA_KERNEL_PATH}/platform/android/*.cpp"
    "${BOYIA_KERNEL_PATH}/platform/arm/*.cpp"
    "${BOYIA_KERNEL_PATH}/platform/graphics/*.cpp"
    "${BOYIA_KERNEL_PATH}/platform/zip/*.cpp"
    "${BOYIA_KERNEL_PATH}/platform/ws/*.cpp"
    "${BOYIA_KERNEL_PATH}/style/*.cpp"
    "${BOYIA_KERNEL_PATH}/util/*.cpp"
    "${BOYIA_KERNEL_PATH}/view/*.cpp"
    "${BOYIA_KERNEL_PATH}/widget/*.cpp"
    "${BOYIA_KERNEL_PATH}/widget/flex/*.cpp"
    "${BOYIA_KERNEL_PATH}/app/*.cpp"
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
    ${kernel_src}
    ${painter_src}
    ${thread_src}
    ${porting_src}
)

target_include_directories(
    boyia
    PRIVATE
    ${BOYIA_KERNEL_PATH}/vm/view
    ${BOYIA_KERNEL_PATH}/vm/core
    ${BOYIA_KERNEL_PATH}/vm/jit
    ${BOYIA_KERNEL_PATH}/vm/json
    ${BOYIA_KERNEL_PATH}/vm/lib
    ${BOYIA_KERNEL_PATH}/vm/dbg
    ${BOYIA_KERNEL_PATH}/app
    ${BOYIA_KERNEL_PATH}/dom
    ${BOYIA_KERNEL_PATH}/gui
    ${BOYIA_KERNEL_PATH}/loader
    ${BOYIA_KERNEL_PATH}/platform/android
    ${BOYIA_KERNEL_PATH}/platform/arm
    ${BOYIA_KERNEL_PATH}/platform/graphics
    ${BOYIA_KERNEL_PATH}/platform/zip
    ${BOYIA_KERNEL_PATH}/platform/ws
    ${BOYIA_KERNEL_PATH}/platform
    ${BOYIA_KERNEL_PATH}/style
    ${BOYIA_KERNEL_PATH}/util
    ${BOYIA_KERNEL_PATH}/view
    ${BOYIA_KERNEL_PATH}/render
    ${BOYIA_KERNEL_PATH}/render/layer
    ${BOYIA_KERNEL_PATH}/widget
    ${BOYIA_KERNEL_PATH}/widget/flex
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
    GLESv2
    android
    jnigraphics
    cjson
    tinyxml
    zip
)