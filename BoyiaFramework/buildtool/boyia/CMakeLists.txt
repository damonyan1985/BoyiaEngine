cmake_minimum_required(VERSION 3.4.1)

project(boyia)

add_definitions(-DANDROID_NDK -DFT2_BUILD_LIBRARY=1 -DPOSIX -DENABLE_BOYIA_ANDROID=1)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -frtti")

set(BOYIA_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../boyia")
set(THIRD_PARTY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty")

if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
    include_directories(${ANDROID_SYSROOT}/usr/include/aarch64-linux-android)
else()
    include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
endif()

file(
    GLOB 
    framework_src
    "${BOYIA_SRC_PATH}/framework/vm/view/*.cpp"
    "${BOYIA_SRC_PATH}/framework/vm/core/*.cpp"
    "${BOYIA_SRC_PATH}/framework/vm/jit/*.cpp"
    "${BOYIA_SRC_PATH}/framework/vm/lib/*.cpp"
    "${BOYIA_SRC_PATH}/framework/vm/json/*.c"
    "${BOYIA_SRC_PATH}/framework/vm/json/*.cpp"
    "${BOYIA_SRC_PATH}/framework/dom/*.cpp"
    "${BOYIA_SRC_PATH}/framework/gui/*.cpp"
    "${BOYIA_SRC_PATH}/framework/loader/*.cpp"
    "${BOYIA_SRC_PATH}/framework/platform/*.cpp"
    "${BOYIA_SRC_PATH}/framework/platform/android/*.cpp"
    "${BOYIA_SRC_PATH}/framework/platform/arm/*.cpp"
    "${BOYIA_SRC_PATH}/framework/platform/graphics/*.cpp"
    "${BOYIA_SRC_PATH}/framework/style/*.cpp"
    "${BOYIA_SRC_PATH}/framework/util/*.cpp"
    "${BOYIA_SRC_PATH}/framework/view/*.cpp"
    "${BOYIA_SRC_PATH}/framework/widget/*.cpp"
    "${BOYIA_SRC_PATH}/framework/widget/flex/*.cpp"
    "${BOYIA_SRC_PATH}/framework/app/*.cpp"
    )

file(
    GLOB 
    graphics_src
    "${BOYIA_SRC_PATH}/graphics/core/*.cpp"
    "${BOYIA_SRC_PATH}/graphics/painter/*.cpp"
    "${BOYIA_SRC_PATH}/graphics/shaders/*.cpp"
    "${BOYIA_SRC_PATH}/graphics/utils/*.cpp"
    "${BOYIA_SRC_PATH}/graphics/vulkan/*.cpp"
    ) 

file(
    GLOB 
    thread_src
    "${BOYIA_SRC_PATH}/thread/*.cpp"
    )

file(
    GLOB 
    porting_src
    "${BOYIA_SRC_PATH}/porting/*.cpp"
    )          

add_library(
    boyia
    SHARED
    ${framework_src}
    ${graphics_src}
    ${thread_src}
    ${porting_src}
)

target_include_directories(
    boyia
    PRIVATE
    ${BOYIA_SRC_PATH}/framework/vm/view
    ${BOYIA_SRC_PATH}/framework/vm/core
    ${BOYIA_SRC_PATH}/framework/vm/jit
    ${BOYIA_SRC_PATH}/framework/vm/json
    ${BOYIA_SRC_PATH}/framework/vm/lib
    ${BOYIA_SRC_PATH}/framework/app
    ${BOYIA_SRC_PATH}/framework/dom
    ${BOYIA_SRC_PATH}/framework/gui
    ${BOYIA_SRC_PATH}/framework/loader
    ${BOYIA_SRC_PATH}/framework/platform/android
    ${BOYIA_SRC_PATH}/framework/platform/arm
    ${BOYIA_SRC_PATH}/framework/platform/graphics
    ${BOYIA_SRC_PATH}/framework/platform
    ${BOYIA_SRC_PATH}/framework/style
    ${BOYIA_SRC_PATH}/framework/util
    ${BOYIA_SRC_PATH}/framework/view
    ${BOYIA_SRC_PATH}/framework/widget
    ${BOYIA_SRC_PATH}/framework/widget/flex
    ${BOYIA_SRC_PATH}/graphics/core
    ${BOYIA_SRC_PATH}/graphics/shaders
    ${BOYIA_SRC_PATH}/graphics/painter
    ${BOYIA_SRC_PATH}/graphics/utils
    ${BOYIA_SRC_PATH}/graphics/vulkan
    ${BOYIA_SRC_PATH}/thread
    ${THIRD_PARTY_PATH}/cjson
    ${THIRD_PARTY_PATH}/tinyxml
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
)