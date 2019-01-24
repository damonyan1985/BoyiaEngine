cmake_minimum_required(VERSION 3.4.1)

project(boyia)

add_definitions(-DANDROID_NDK -DFT2_BUILD_LIBRARY=1 -DPOSIX)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -frtti")

set(EXTERNAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../External")

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
    "framework/byvm/app/*.cpp"
    "framework/byvm/binds/*.cpp"
    "framework/byvm/core/*.cpp"
    "framework/byvm/jit/*.cpp"
    "framework/byvm/lib/*.cpp"
    "framework/byvm/json/*.c"
    "framework/byvm/json/*.cpp"
    "framework/dom/*.cpp"
    "framework/gui/*.cpp"
    "framework/loader/*.cpp"
    "framework/platform/android/*.cpp"
    "framework/platform/arm/*.cpp"
    "framework/platform/graphics/*.cpp"
    "framework/style/*.cpp"
    "framework/util/*.cpp"
    "framework/view/*.cpp"
    "framework/widget/*.cpp"
    "framework/widget/flex/*.cpp"
    )

file(
    GLOB 
    graphics_src
    "graphics/core/*.cpp"
    "graphics/painter/*.cpp"
    "graphics/shaders/*.cpp"
    "graphics/utils/*.cpp"
    "graphics/vulkan/*.cpp"
    ) 

file(
    GLOB 
    network_src
    "network/font/*.cpp"
    "network/image/*.cpp"
    "network/ipc/*.cpp"
    "network/test/*.cpp"
    "network/threadpool/*.cpp"
    "network/*.cpp"
    "network/proxy/ptcp/*.cpp"
    "network/proxy/server/*.cpp"
    )

file(
    GLOB 
    imessage_src
    "imessage/ptcp/*.cpp"
    )    

file(
    GLOB 
    porting_src
    "porting/*.cpp"
    )          

add_library(
    boyia
    SHARED
    ${framework_src}
    ${graphics_src}
    ${network_src}
    ${imessage_src}
    ${porting_src}
)

target_include_directories(
    boyia
    PRIVATE
    framework/byvm/app
    framework/byvm/binds
    framework/byvm/core
    framework/byvm/jit
    framework/byvm/json
    framework/byvm/lib
    framework/dom
    framework/gui
    framework/loader
    framework/platform/android
    framework/platform/arm
    framework/platform/graphics
    framework/platform
    framework/style
    framework/util
    framework/view
    framework/widget
    framework/widget/flex
    graphics/core
    graphics/shaders
    graphics/painter
    graphics/utils
    graphics/vulkan
    network
    network/font
    network/image
    network/ipc
    network/threadpool
    network/proxy/ptcp
    network/proxy/server
    ${EXTERNAL_PATH}/curl/include
    ${EXTERNAL_PATH}/jpeg
    ${EXTERNAL_PATH}/lpng/include
    ${EXTERNAL_PATH}/ft2/include
    ${EXTERNAL_PATH}/trace
)

find_library(
    boyia_link_lib
    log
    android
)

target_link_libraries(
    boyia
    curl
    jpeg
    png
    ft2
    log
    EGL
    GLESv3
    android
    jnigraphics
)